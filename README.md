# Parallel Computing: Boids Simulation with OpenMP

Sequential and parallel implementations of the **Boids algorithm**, developed in C using **OpenMP**.

## Prerequisites

### C Libraries

* **Linux:**

  ```bash
  sudo apt update && sudo apt install -y libx11-dev libxcursor-dev libxinerama-dev libxrandr-dev libxi-dev libgl1-mesa-dev libomp-dev
  git clone --depth 1 --branch 5.0 https://github.com/raysan5/raylib.git
  cd raylib/src && make PLATFORM=PLATFORM_DESKTOP -j$(nproc) && sudo make install
  ```

* **macOS:**

  ```bash
  brew install cmake raylib libomp
  ```

* **Windows:**

  ```bash
  vcpkg install raylib:x64-windows
  ```

The project uses **uv** to manage the dependencies required by the benchmarking and plotting scripts.

* **macOS / Linux:**

  ```bash
  curl -LsSf https://astral.sh/uv/install.sh | sh
  ```

* **Windows:**

  ```powershell
  powershell -c "irm https://astral.sh/uv/install.ps1 | iex"
  ```

---

## Compilation

### Configuration

* **macOS / Linux:**

  ```bash
  cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
  ```

* **Windows:**

  ```powershell
  cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"
  ```

### Build

```bash
cmake --build cmake-build-release --config Release --parallel 4
```

### Main Targets

* `boids_seq_aos` / `boids_seq_soa`: Sequential implementations.
* `boids_par_aos_histo` / `boids_par_soa_histo`: Parallel implementations using reduction through local histograms.
* `boids_par_aos_atomic` / `boids_par_soa_atomic`: Parallel implementations using atomic operations.
* `boids_O0`: SoA Histo implementation compiled without compiler optimizations.

---

## Command-Line Usage

Each executable accepts the following parameters:

* `--mode [gui|benchmark|validate]`
* `--boids N`
* `--threads N`
* `--steps N`
* `--runs N`

### Example

```bash
./boids_par_soa_histo --mode gui --boids 10000 --threads 8
```

---

## Analysis Scripts

The scripts used to collect data for performance analysis and demos are located in the `scripts/` directory.

### Benchmarks

* **Full Benchmark:** Runs all implementations for different numbers of boids and threads.

  ```bash
  uv run python run_all_implementations_benchmarks.py
  uv run python weak_scaling_benchmarks.py
  ```

* **Scheduling Analysis:** Evaluates the performance of `static`, `dynamic`, and `guided` scheduling.

  ```bash
  uv run python run_schedule_benchmarks.py
  ```

### Plots

After collecting the benchmark data, which is saved in `results/dumps/`, generate the plots with:

```bash
uv run python analisi_dati.py
```

### Physical Validation

Verifies that the parallel implementations produce results identical to the sequential implementation.

```bash
uv run python run_validation_all.py
```

### Demos

* **Graphical Execution:** Runs a graphical test of the simulation.

  ```bash
  uv run python run_gui.py
  ```

* **Comparison Demo:** Compares a sequential implementation with a parallel implementation.

  ```bash
  uv run python run_demo.py
  ```

---
