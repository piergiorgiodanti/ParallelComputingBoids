#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stddef.h>

#ifdef _WIN32
    #define NOGDI
    #define NOUSER
    #define NOMB
    #include <windows.h>
    #include <process.h>
    #include <malloc.h>

    #define ALIGNED_ALLOC(alignment, size) _aligned_malloc((size), (alignment))
    #define ALIGNED_FREE(ptr) _aligned_free(ptr)
    #define RAND_R(seed) rand()
#else
    #include <unistd.h>

    #define ALIGNED_ALLOC(alignment, size) aligned_alloc((alignment), (size))
    #define ALIGNED_FREE(ptr) free(ptr)
    #define RAND_R(seed) rand_r(seed)
#endif

#include "boids_manager.h"
#include "boids_types.h"

#ifdef _OPENMP
    #include <omp.h>
#endif

#if defined(__APPLE__)
    #include <sys/sysctl.h>
#endif

size_t get_cache_line_size() {
#if defined(__APPLE__)
    size_t line_size = 0;
    size_t sizeof_line_size = sizeof(line_size);
    if (sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, NULL, 0) == 0) {
        return line_size;
    }
    return 64;

#elif defined(_WIN32)
    size_t line_size = 64;
    DWORD buffer_size = 0;
    GetLogicalProcessorInformation(0, &buffer_size);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buffer =
        (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);

    if (buffer != NULL && GetLogicalProcessorInformation(buffer, &buffer_size)) {
        for (DWORD i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
            if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
                line_size = buffer[i].Cache.LineSize;
                break;
            }
        }
    }
    free(buffer);
    return line_size;

#else
    long size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    return (size > 0) ? (size_t)size : 64;
#endif
}

void init_boids(const BoidSystem* boids, const int num_boids) {
    unsigned int master_seed = 42;
    for (int i = 0; i < num_boids; i++) {
        // Usiamo la macro RAND_R che si adatta al sistema operativo
        const float start_x = MARGIN_X + (RAND_R(&master_seed) % (SCREEN_WIDTH - 2 * MARGIN_X));
        const float start_y = MARGIN_Y + (RAND_R(&master_seed) % (SCREEN_HEIGHT - 2 * MARGIN_Y));
        const float angle = (float)(RAND_R(&master_seed) % 360) * DEG2RAD;
        const float speed = MIN_SPEED + ((float)RAND_R(&master_seed) / (float)RAND_MAX) * (MAX_SPEED - MIN_SPEED);
        const float start_vx = cosf(angle) * speed;
        const float start_vy = sinf(angle) * speed;
        B_X(boids, i) = start_x;
        B_Y(boids, i) = start_y;
        B_VX(boids, i) = start_vx;
        B_VY(boids, i) = start_vy;
    }
}

void init_boids_system(BoidSystem *b, int num_boids) {
    b->count = num_boids;

    size_t alignment = get_cache_line_size();
    if (alignment <= 0) {
        alignment = 64;
    }

    #ifdef USE_SOA
    size_t size_bytes = sizeof(float) * num_boids;
    size_t aligned_size = ((size_bytes + alignment - 1) / alignment) * alignment;

    b->x  = (float *) ALIGNED_ALLOC(alignment, aligned_size);
    b->y  = (float *) ALIGNED_ALLOC(alignment, aligned_size);
    b->vx = (float *) ALIGNED_ALLOC(alignment, aligned_size);
    b->vy = (float *) ALIGNED_ALLOC(alignment, aligned_size);

    if (!b->x || !b->y || !b->vx || !b->vy) {
        fprintf(stderr, "Errore di allocazione SoA\n");
        exit(-1);
    }
    #else
    size_t size_bytes = sizeof(Boid) * num_boids;
    size_t aligned_size = ((size_bytes + alignment - 1) / alignment) * alignment;

    b->data = (Boid *) ALIGNED_ALLOC(alignment, aligned_size);

    if (!b->data) {
        fprintf(stderr, "Errore di allocazione AoS\n");
        exit(-1);
    }
    #endif
}

void free_boids_system(BoidSystem *b) {
    if (b == NULL) return;

    #ifdef USE_SOA
    ALIGNED_FREE(b->x);
    ALIGNED_FREE(b->y);
    ALIGNED_FREE(b->vx);
    ALIGNED_FREE(b->vy);
    b->x = b->y = b->vx = b->vy = NULL;
    #else
    ALIGNED_FREE(b->data);
    b->data = NULL;
    #endif
}

void free_simulation_context(const SimulationContext *ctx) {
    if (ctx == NULL) return;

    free(ctx->counting_cell);
    free(ctx->cell_offsets);
    free(ctx->sorted_ind);
    free(ctx->temp_offsets);

    #ifdef _OPENMP
    if (ctx->local_histograms) {
        for (int t = 0; t < ctx->num_threads; t++) {
            ALIGNED_FREE(ctx->local_histograms[t]);
            ALIGNED_FREE(ctx->local_offsets[t]);
        }
        free(ctx->local_histograms);
        free(ctx->local_offsets);
    }
    #endif
}

void init_simulation_context(SimulationContext *ctx, int num_boids) {
    #ifdef _OPENMP
    ctx->num_threads = omp_get_max_threads();
    #else
    ctx->num_threads = 1;
    #endif

    const float scale = sqrtf(1000.0f / (float)num_boids);
    const float v_range = 150.0f * scale;
    ctx->protected_range_sq = (40.0f * scale) * (40.0f * scale);
    ctx->visual_range_sq = v_range * v_range;
    ctx->left_margin = MARGIN_X; ctx->right_margin = SCREEN_WIDTH - MARGIN_X;
    ctx->top_margin = MARGIN_Y; ctx->bottom_margin = SCREEN_HEIGHT - MARGIN_Y;
    ctx->grid_cols = (SCREEN_WIDTH / (int)v_range) + 1;
    ctx->grid_rows = (SCREEN_HEIGHT / (int)v_range) + 1;
    ctx->total_cells = ctx->grid_cols * ctx->grid_rows;

    ctx->counting_cell = calloc(ctx->total_cells, sizeof(int));
    ctx->cell_offsets = malloc((ctx->total_cells + 1) * sizeof(int));
    ctx->sorted_ind = malloc(num_boids * sizeof(int));

    ctx->temp_offsets = malloc(ctx->total_cells * sizeof(int));

    #if defined(_OPENMP) && !defined(USE_ATOMIC)
    ctx->local_histograms = malloc(ctx->num_threads * sizeof(int *));
    ctx->local_offsets = malloc(ctx->num_threads * sizeof(int *));

    size_t cache_line = get_cache_line_size();
    if (cache_line <= 0) cache_line = 64;

    size_t size_bytes = ctx->total_cells * sizeof(int);
    size_t aligned_size = ((size_bytes + cache_line - 1) / cache_line) * cache_line;

    for (int t = 0; t < ctx->num_threads; t++) {
        ctx->local_histograms[t] = ALIGNED_ALLOC(cache_line, aligned_size);
        memset(ctx->local_histograms[t], 0, aligned_size);
        ctx->local_offsets[t] = ALIGNED_ALLOC(cache_line, aligned_size);
    }

    #else
    ctx->local_histograms = NULL; ctx->local_offsets = NULL;
    #endif
}