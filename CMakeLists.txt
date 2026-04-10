cmake_minimum_required(VERSION 3.20)
project(boids C)

set(CMAKE_C_STANDARD 11)

include_directories(include /usr/local/include /opt/homebrew/include)
link_directories(/usr/local/lib /opt/homebrew/lib)

# Gestione dipendenze
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LIBS raylib dl rt X11 m pthread)
elseif(APPLE)
    set(LIBS raylib)
else()
    find_package(raylib CONFIG REQUIRED)
    set(LIBS raylib)
endif()

find_package(OpenMP REQUIRED)

# Flag ottimizzazioni
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2 /fp:fast /W3 /GL -openmp:llvm")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -ffast-math -flto -Wall")
endif()

set(COMMON_SOURCES
        src/main.c
        src/boids_simulation.c
        src/boids_core.c
        src/boids_manager.c
)

# TARGETS
add_executable(boids_seq_aos ${COMMON_SOURCES})
target_link_libraries(boids_seq_aos PRIVATE ${LIBS})

add_executable(boids_seq_soa ${COMMON_SOURCES})
target_compile_definitions(boids_seq_soa PRIVATE USE_SOA)
target_link_libraries(boids_seq_soa PRIVATE ${LIBS})

add_executable(boids_par_aos_histo ${COMMON_SOURCES})
target_link_libraries(boids_par_aos_histo PRIVATE OpenMP::OpenMP_C ${LIBS})

add_executable(boids_par_soa_histo ${COMMON_SOURCES})
target_compile_definitions(boids_par_soa_histo PRIVATE USE_SOA)
target_link_libraries(boids_par_soa_histo PRIVATE OpenMP::OpenMP_C ${LIBS})

# Versione soa_histo non ottimizzata
add_executable(boids_O0 ${COMMON_SOURCES})
target_compile_definitions(boids_O0 PRIVATE USE_SOA)
# Disabilitiamo le ottimizzazioni in base al compilatore
if(MSVC)
    target_compile_options(boids_O0 PRIVATE /Od)
else()
    target_compile_options(boids_O0 PRIVATE -O0)
endif()
target_link_libraries(boids_O0 PRIVATE OpenMP::OpenMP_C ${LIBS})

add_executable(boids_par_soa_atomic ${COMMON_SOURCES})
target_compile_definitions(boids_par_soa_atomic PRIVATE USE_SOA USE_ATOMIC)
target_link_libraries(boids_par_soa_atomic PRIVATE OpenMP::OpenMP_C ${LIBS})

add_executable(boids_par_aos_atomic ${COMMON_SOURCES})
target_compile_definitions(boids_par_aos_atomic PRIVATE USE_ATOMIC)
target_link_libraries(boids_par_aos_atomic PRIVATE OpenMP::OpenMP_C ${LIBS})