# Parallel Computing: Boids Simulation OpenMP

Implementazione sequenziale e parallela dell'algoritmo Boids, sviluppato in C con **OpenMP**.

## Prerequisiti

### Librerie C
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
  
Il progetto utilizza **uv** per gestire le dipendenze degli script di benchmark e plotting.
* **macOS / Linux:**
  ```bash
  curl -LsSf https://astral.sh/uv/install.sh | sh
  ```
* **Windows:**
  ```powershell
  powershell -c "irm https://astral.sh/uv/install.ps1 | iex"
  ```

---

## Compilazione
### Configurazione
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

### Target principali:
* `boids_seq_aos` / `boids_seq_soa`: Versioni sequenziali.
* `boids_par_aos_histo` / `boids_par_soa_histo`: Parallelismo con riduzione tramite istogrammi locali.
* `boids_par_aos_atomic` / `boids_par_soa_atomic`: Parallelismo tramite istruzioni atomiche.
* `boids_O0`: Versione SoA Histo senza ottimizzazioni del compilatore.

---

## Utilizzo riga di comando

Ogni eseguibile accetta i seguenti parametri:
- `--mode [gui|benchmark|validate]`
- `--boids N`
- `--threads N`
- `--steps N`
- `--runs N`

### Esempio
```bash
./boids_par_soa_histo --mode gui --boids 10000 --threads 8
```

---

## Scripts per analisi

Gli script per raccogliere i dati per le analisi si trovano nella cartella `scripts/`.

### Benchmark
* **Benchmark Completo:** Esegue tutti i target per diversi numeri di boids e thread.
    ```bash
    uv run python run_all_implementations_benchmarks.py
    uv run python weak_scaling_benchmarks.py
    ```
* **Analisi Scheduling:** Valuta lo scheduling con `static`, `dynamic` e `guided`.
    ```bash
    uv run python run_schedule_benchmarks.py
    ```
* **Validazione Fisica:** Verifica che le versioni parallele producano risultati identici alla sequenziale.
    ```bash
    uv run python run_validation_all.py
    ```
* **Esecuzione Demo:** Avvio test della grafica.
    ```bash
    uv run python run_demo.py
    ```

### Grafici
Dopo aver raccolto i dati (che verranno salvati in `results/dumps/`), generare i plot con:
```bash
uv run python analisi_dati.py
```

---
