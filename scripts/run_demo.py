import subprocess
import os
import sys
import platform

BUILD_DIR = "../cmake-build-release"
OUT_FILE = "../results/dumps/tmp.csv"

TARGET_SEQ = "boids_seq_soa"
TARGET_PAR = "boids_par_soa_histo"

BOIDS = 10000
THREADS = 8
STEPS = 500
RUNS = 1
OMP_SCHEDULE = "dynamic,64"

def run_target(target_base_name):
    ext = ".exe" if platform.system() == "Windows" else ""
    target_name = f"{target_base_name}{ext}"
    target_path = os.path.join(BUILD_DIR, target_name)

    if not os.path.isfile(target_path):
        print(f">>> [ERRORE] Binario non trovato: {target_path}")
        return False

    print(f"\n>>> [RUNNING] {target_name}...")

    env = os.environ.copy()
    env["OMP_SCHEDULE"] = OMP_SCHEDULE

    cmd = [
        target_path,
        "--mode", "benchmark",
        "--boids", str(BOIDS),
        "--threads", str(THREADS) if "par" in target_base_name else "1",
        "--steps", str(STEPS),
        "--runs", str(RUNS),
        "--out", OUT_FILE
    ]

    try:
        subprocess.run(cmd, env=env, check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f">>> [ERRORE] Fallimento {target_name} (Exit Code: {e.returncode})")
        return False

def main():
    print("RUN DEMO - Sequential vs Parallel 8 thread SoA - 10000 boids - 500 steps")

    os.makedirs(os.path.dirname(OUT_FILE), exist_ok=True)

    run_target(TARGET_SEQ)

    run_target(TARGET_PAR)

    os.remove(OUT_FILE)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterruzione da parte dell'utente.")
        sys.exit(0)