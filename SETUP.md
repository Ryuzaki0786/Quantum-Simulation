# Setup — build & run

Everything here targets the CUDA backend, which is the only part that needs a GPU. The CPU solvers (ODE, PDE, quantum circuit) build with a plain C++17 compiler; the two files below are the ones people actually want to run, since they produce the parity check and the residency benchmark.

There is **no local NVIDIA GPU** in this project — all CUDA work runs on a rented Google Colab T4. If you have your own CUDA-capable GPU, the same commands work; just skip the Colab steps.

---

## What you need

- **GPU:** an NVIDIA card with compute capability **sm_75** (Tesla T4). On Colab: *Runtime → Change runtime type → T4 GPU*. Confirm with `nvidia-smi` — it should report a Tesla T4.
- **Toolchain:** `nvcc` (CUDA Toolkit) and a host `g++` with OpenMP. Both are preinstalled on a Colab GPU runtime.
- **Files that travel together:** `quantum_register.h`, `matrix.h`, `vector.h`, plus the `.cu` under test. `quantum_register.h` includes the other two, so all three must be reachable at compile time.

---

## Get the code

Cloning the whole repo is the recommended path — it pulls every file at once and sidesteps the raw-fetch corruption gotcha described at the bottom.

```bash
git clone https://github.com/Ryuzaki0786/Quantum-Simulation.git
cd Quantum-Simulation
```

In a Colab cell, prefix shell commands with `!` (e.g. `!git clone ...`).

---

## Build & run

Run these from the repository root. The `-Iinclude` flag tells `nvcc` where the headers live, so the `.cu` files find them regardless of the `cuda/` vs `include/` split.

**Parity oracle** — confirms the CPU and CUDA gate paths agree amplitude-for-amplitude:

```bash
nvcc -arch=sm_75 -Xcompiler -fopenmp -Iinclude cuda/parity_test.cu -o parity && ./parity
```

**Residency benchmark** — the headline result (per-gate vs. resident timing):

```bash
nvcc -arch=sm_75 -Xcompiler -fopenmp -Iinclude cuda/benchmark_residency.cu -o bench && ./bench
```

Two flags worth understanding:

- **`-arch=sm_75`** targets the T4's architecture. On a different GPU, set this to your card's compute capability, or the kernel may fail to launch.
- **`-Xcompiler -fopenmp`** forwards `-fopenmp` to the host `g++` underneath `nvcc`. The headers carry `#pragma omp` directives and include `<omp.h>`; without this flag the pragmas are silently ignored and the CPU paths run single-threaded.

### One include-path caveat

`benchmark_residency.cu` includes its headers by bare name (`#include "quantum_register.h"`), which resolves cleanly with `-Iinclude`. `parity_test.cu` currently includes them as `#include "../quantum_register.h"`, which assumes the headers sit one directory *above* the file — they don't, in this layout. Change those two lines in `parity_test.cu` to bare names:

```cpp
#include "quantum_register.h"   // was "../quantum_register.h"
#include "matrix.h"             // was "../matrix.h"
```

Then the `-Iinclude` command above builds it the same way as the benchmark. (Aligning the two files on bare includes is the clean fix; do it once in the source and both build uniformly.)

---

## Expected output

**Parity** — the difference is exactly zero because both paths run identical arithmetic in identical order on identical doubles:

```
qubits = 12, amplitudes = 4096
max amplitude difference (CPU vs CUDA) = 0.000e+00
PARITY PASS
```

**Residency benchmark** — the per-gate column grows roughly linearly with gate count while the resident column stays nearly flat, so the speedup climbs with circuit depth. Exact milliseconds vary run to run, but the shape and the ~95× tail should reproduce:

```
   gates    per-gate (ms)    resident (ms)    speedup
       1           53.827           53.658       1.00x
       2           94.389           51.797       1.82x
       4          185.855           56.360       3.30x
       8          398.019           64.245       6.20x
      16          746.408           52.418      14.24x
      32         1511.318           58.072      26.02x
      64         3679.334           68.852      53.44x
     128         6206.387           98.171      63.22x
     256        12834.275          133.783      95.93x
```

The benchmark discards a warm-up call first, so the one-time CUDA context-initialization cost doesn't land on the N=1 measurement.

---

## Gotcha: raw-fetch file corruption (429)

If you fetch individual headers from GitHub's raw endpoint (rather than cloning), rate-limiting can return an **HTTP 429** error *page* that gets written into the file in place of the source. The file then starts with HTML instead of `#ifndef`, and the compiler emits a cascade of nonsense errors — undefined `__mbstate_t`, `std` has no member `complex`, and dozens more — all downstream of that first garbage line.

Before building, verify each header actually contains code:

```bash
head -3 quantum_register.h    # should start with: #ifndef QUANTUM_REGISTER_H
```

If you see `<!DOCTYPE html>` or a rate-limit message, re-fetch (or just clone the repo). Cloning avoids this entirely, which is why it's the recommended path above.
