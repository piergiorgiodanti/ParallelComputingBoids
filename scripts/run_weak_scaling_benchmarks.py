import subprocess
import os
import sys

BUILD_DIR = "../cmake-build-release"
OUT_CSV = "../results/dumps/weak_scaling_benchmarks.csv"

TARGETS = [
    "boids_par_soa_histo",
]

WEAK_SCALING_CONFIGS = [
    (1250, 1),
    (2500, 2),
    (5000, 4),
    (10000, 8),
    (20000, 16)
]

STEPS = 1000
RUNS = 5

OMP_ENV = os.environ.copy()
OMP_ENV["OMP_SCHEDULE"] = "dynamic,64"

def main():
    print("="*68)
    print(f" START: WEAK SCALING BENCHMARKS ")
    print(f" Configurazione: {STEPS} steps, {RUNS} runs, Costante: 1250 boids/core")
    print("="*68)

    os.makedirs(os.path.dirname(OUT_CSV), exist_ok=True)

    for target in TARGETS:
        target_path = os.path.join(BUILD_DIR, target)

        if not os.path.isfile(target_path):
            print(f">>> [SKIP] {target} non trovato in {BUILD_DIR}")
            continue

        print("-" * 60)
        print(f">>> [TESTING TARGET]: {target}")

        for boids, threads in WEAK_SCALING_CONFIGS:

            if "seq" in target and threads > 1:
                continue

            print(f"\n>>> [START] Weak Scaling: Boids={boids}, Threads={threads}")

            cmd = [
                target_path,
                "--mode", "benchmark",
                "--boids", str(boids),
                "--threads", str(threads),
                "--steps", str(STEPS),
                "--runs", str(RUNS),
                "--out", OUT_CSV
            ]

            try:
                subprocess.run(cmd, env=OMP_ENV, check=True)
            except subprocess.CalledProcessError:
                print(f"    [ERRORE] {target} crashato. Config: B:{boids} T:{threads}")
            except Exception as e:
                print(f"    [ERRORE INASPETTATO]: {e}")

        print(f">>> [DONE] {target} completato.")

    print("="*68)
    print(f" FINISH: Dati salvati in {OUT_CSV}")
    print("="*68)

if __name__ == "__main__":
    main()