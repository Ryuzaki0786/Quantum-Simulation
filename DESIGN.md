# Design & derivations

This document explains *why* the project is built the way it is — the physics and math behind each phase, and the design decisions that followed from them. The [README](README.md) covers what the project is and its headline result; this is the deeper layer for a reader who wants to see the reasoning, not just the outcome.

It follows the project's own arc, read three ways at once: **ODE → PDE → quantum**, **explicit → implicit**, and **serial → parallel → GPU**. Each phase below is a step along that spine.

---

## Phase 1 — Linear algebra core and ODE steppers

**The math.** An ODE solver advances a state `y` under `dy/dt = f(t, y)`. Explicit Euler takes one slope sample per step, `y_{n+1} = y_n + Δt·f(t_n, y_n)`, and carries global error O(Δt) — first order. RK4 takes a weighted average of four slopes, `y_{n+1} = y_n + (k1 + 2k2 + 2k3 + k4)·Δt/6`; the `(1, 2, 2, 1)/6` weighting is chosen precisely so the Taylor expansion of the error cancels through fourth order, giving global error O(Δt⁴). On `dy/dt = −y` at `Δt = 0.01`, Euler's error was 1.8e-3 and RK4's was 3.09e-11 — the order gap made concrete at a single step size. (This is a one-point comparison, not a convergence study; the defensible claim is the *order*, O(Δt⁴) vs O(Δt), not the raw ratio.)

**Design decisions.**

*Abstract base with a pure virtual `step()`.* `Solver<T>` declares `step()` as `= 0`. This forces every derived solver to implement it, and makes accidentally instantiating a bare `Solver<T>` a compile error rather than a silent no-op. A concrete default would let a misspelled or missing override fall through to base behavior invisibly — the pure virtual turns that class of bug into a compile-time failure. The base also declares a `virtual` destructor (`= default`): without it, `delete` through a `Solver<T>*` pointing at an `RK4Solver<T>` would run only the base destructor and leak whatever the derived class held.

*Templated primitives.* `Vector<T>` and `Matrix<T>` are templated so the same code carries `double` for classical solvers and `std::complex<double>` for quantum work later. The core is deliberately small and shared — the `Matrix<Complex>` that represents a quantum gate is the same type that represents a Crank-Nicolson update operator two phases later.

*LU as the linear-solve substrate.* `Matrix<T>::solve(b)` factors `A = LU`, then forward-substitutes `Ly = b` and back-substitutes `Ux = y`. This exists in Phase 1 with no immediate consumer — it's built because the implicit PDE solver in Phase 3 will need exactly this, and building it against a known-answer test now is cheaper than debugging it inside a physics solver later.

*Higher-order ODEs as first-order systems.* Any Nth-order ODE becomes N coupled first-order ODEs by introducing a variable per derivative. The damped oscillator `x'' = −(c/m)x' − (k/m)x` becomes the 2D system `[x, v]` with `x' = v`, `v' = −(c/m)v − (k/m)x`. The solver never sees "second order" — it just steps a vector forward, so state dimensionality is the only thing that changes.

---

## Phase 2 — Classical PDEs: wave and heat

**The math.** The wave equation `∂²u/∂t² = c²∂²u/∂x²` is solved with a leapfrog (central-difference-in-time) scheme. Its stability is governed by the **CFL condition**: the numerical domain of dependence must contain the physical one, which bounds the time step by the grid spacing and wave speed (`c·Δt/Δx ≤ 1`). The heat equation `∂u/∂t = α∂²u/∂x²` uses an explicit forward-time stencil, stable only when the diffusion number `α·Δt/Δx²` stays below a fixed bound.

**Design decisions.**

*Stability numbers are derived and checked, never passed in.* The CFL and diffusion limits are properties of the discretization, not user inputs. Exposing them as parameters would invite a caller to set a value that silently produces a blow-up. They're computed from the grid and asserted internally instead.

*The spatial grid is non-negotiable.* An early conceptual error was treating the heat equation as if it had "no concept of length." It does: diffusion happens *in space*, so `Δx = length/(n_points − 1)` is intrinsic — the same spatial-grid structure the wave equation needs. Both are PDEs in space and time; the design treats them uniformly rather than special-casing heat.

Both solvers are validated physically (wave propagation and reflection, diffusion smoothing) under a Catch2 suite, not just checked for "runs without crashing."

---

## Phase 3 — The Schrödinger equation via Crank-Nicolson

This is the phase where the explicit → implicit transition earns itself, and it's the strongest single piece of physics reasoning in the project.

**The math.** The time-dependent Schrödinger equation rearranges to `∂ψ/∂t = (iℏ/2m)∂²ψ/∂x² − (i/ℏ)Vψ` — structurally the heat equation, but with an **imaginary** diffusion coefficient. That one factor of `i` changes everything. Do a von Neumann stability analysis by substituting a single Fourier mode `e^{ikx}` and asking how its amplitude factor `g` behaves:

- **Explicit forward Euler** gives `|g| = |1 − iθ| = √(1 + θ²) > 1` for every θ. It is *unconditionally* unstable — no time step saves it, because the instability is in the scheme, not the step size.
- **Backward Euler** gives `|g| = 1/√(1 + θ²) < 1` — stable, but it *decays*. For the heat equation decay is physical; for Schrödinger it destroys probability, which must be conserved.
- **Crank-Nicolson** averages present and future (trapezoidal in time), giving `g = (2 − iθ)/(2 + iθ)` — a complex number divided by its own conjugate, so `|g| = 1` *exactly*. The scheme is unitary: it rotates the phase without changing the magnitude, so `∫|ψ|² = 1` is preserved by construction.

In matrix form this is `A·ψⁿ⁺¹ = B·ψⁿ` with `A` and `B` complex conjugates — an implicit update that requires solving a linear system every time step.

**Design decisions.**

*Crank-Nicolson over any explicit scheme.* The choice is not about accuracy, it's about the conservation law. An explicit scheme cannot be both stable and probability-conserving here; CN is unconditionally stable *and* unitary. The price is one linear solve per step — which is exactly why the LU machinery was built in Phase 1.

*Factor the update operator once, reuse it every step.* `A` is constant in time (the potential and grid don't change), so its LU factorization is computed once and reused across all steps, turning a per-step O(n³) decomposition into an O(n²) solve. (The speedup here is reasoned from complexity, not benchmarked — treat the specific figure as an estimate, not a measurement.)

*Trust the conserved quantity over the picture.* During barrier reflection, a local `|ψ|²` spike (~2.85) initially looked like a bug; it was legitimate constructive interference, and the total probability was conserved throughout. The lesson — "the number is the truth, not the picture" — became a working principle: validate against the invariant (`∫|ψ|² = 1`), not against visual intuition about the plot.

---

## Phase 4 — Quantum circuit simulation

**The math.** A qubit is a 2-element complex vector `[α, β]` with `|α|² + |β|² = 1`. A gate is a 2×2 **unitary** matrix (`U†U = I`), and unitarity is not an arbitrary requirement — it is exactly the property that preserves vector norm, which is preserving total probability, which is the same conservation law as the Schrödinger equation, now discretized onto qubits. An n-qubit register has `2ⁿ` joint amplitudes: possibilities multiply, they don't add.

The pivotal fact is **entanglement**. The Bell state `(|00⟩ + |11⟩)/√2` cannot be written as a product of two single-qubit states. Proof by contradiction: if it factored as `(a₀|0⟩ + a₁|1⟩) ⊗ (b₀|0⟩ + b₁|1⟩)`, then the `|01⟩` amplitude `a₀b₁ = 0` forces `b₁ = 0`, but the `|11⟩` amplitude `a₁b₁ ≠ 0` forces `b₁ ≠ 0` — a contradiction.

**Design decisions.**

*Store the full `2ⁿ` state vector, not a list of qubits.* This is a design decision **forced by physics**. If states always factored, you could store n independent qubits in O(n) space. Entanglement proves they don't factor, so the register must hold the entire joint amplitude vector. The exponential memory cost is not an implementation weakness — it is the honest cost of representing entangled state, and it's why a 20-qubit register is already ~1M amplitudes.

*Apply single-qubit gates by index pairing.* A gate on target qubit `t` acts on pairs of basis states that differ only in bit `t`: for each index `i` with that bit clear, `j = i | (1<<t)` is its partner, and the 2×2 gate mixes `state[i]` and `state[j]`. This touches every amplitude exactly once and needs no full `2ⁿ × 2ⁿ` matrix.

*Measurement is Born-rule sampling with collapse.* `P(0)` on a target is the sum of `|amplitude|²` over basis states where that bit is 0; a uniform draw against `P(0)` picks the outcome, amplitudes inconsistent with it are zeroed, and the state is renormalized. Because measurement carries RNG and mutates state, a separate non-collapsing `probabilityZero()` is provided for anything that needs the probability without the side effects (used heavily for benchmarking in Phase 5).

---

## Phase 5 — Performance: OpenMP and the roofline

**The model.** The **roofline** model says a kernel is limited either by compute throughput or by memory bandwidth, depending on its *arithmetic intensity* (operations per byte moved). Phase 5 deliberately collects one example of each regime rather than optimizing a single kernel in isolation:

- **Compute-bound** — the gate sweep does enough arithmetic per amplitude to scale with cores: **3.2×** under OpenMP.
- **Intermediate** — the Born-rule reduction, **2.5×** (1073.86 ms → 426.219 ms at n=24, 16 threads).
- **Memory-bound** — the heat stencil moves a lot of data per flop, so it is bandwidth-limited; OpenMP makes it *slower* (<1×) because extra threads add contention without adding usable bandwidth. This is kept as the honest contrast case, not hidden.

A naive GPU port then introduces the **transfer-bound** regime (the kernel runs in ~0.339 ms but the host↔device round-trip is ~8.093 ms, ~96% of the time on the bus) — which is the problem Phase 6 exists to solve.

**Design decisions.**

*Parallelization must be physics-invisible — that's the correctness oracle.* Any parallel version must produce bit-identical physics to the serial one: probability still sums to 1, Bell correlation still 1000/1000, teleportation still 1000/1000. A divergence between 1-thread and N-thread output is the signature of a race. This oracle runs *before* any timing, because a fast wrong answer is worthless.

*Benchmark the non-collapsing probability path.* `measureQubit` draws on an RNG and collapses state, so it can't be cleanly timed or compared across thread counts. `probabilityZero()` computes the same `P(0)` sum with `reduction(+:p0)` and no side effects — a design choice that exists specifically to make the reduction measurable.

*Launch geometry.* The CUDA kernel launches `threads = 256`, `blocks = (size + threads − 1) / threads` — ceiling division so the last partial block is covered, with an `idx >= size` guard inside the kernel to kill the overflow threads. For `size = 2²⁰`, that's 4096 blocks.

One lesson worth recording: a small smoke test printing thread indices came back *in order*, which looked like an ordering guarantee. It wasn't — 8 threads fit in a single warp with no contention, so nothing exposed the reordering. "No ordering guarantee" means you can rely on neither order nor disorder; at `2²⁰` across many warps it would scramble.

---

## Phase 6 — CUDA as a real backend: the two-boundary adapter and residency

**The problem.** A naive GPU gate is transfer-bound (Phase 5). Making the GPU actually pay off requires keeping the state **resident** on the device across many gates, so one transfer is amortized over a whole circuit instead of paid per gate.

**Design decisions.**

*The adapter crosses two independent boundaries.* The register stores state as interleaved `std::complex<double>` (AoS: `re0, im0, re1, im1, …`); the kernel wants separated `re[]` and `im[]` arrays of plain doubles (SoA). So `applyGateToQubitCUDA` crosses a **layout** boundary (AoS ↔ SoA, a cheap O(N) host-side deinterleave/re-interleave) *and* a **memory-space** boundary (host RAM ↔ device global memory, the expensive PCIe copy). Keeping these two mentally separate matters because they have different costs and get eliminated at different times — residency removes the second, not the first.

*Split-double SoA over `cuDoubleComplex`.* Storing real and imaginary parts as separate `double` arrays means the device arithmetic is plain-double arithmetic — the only genuinely new thing in the first port is CUDA mechanics, not a new complex type with its own rounding behavior on the device. SoA also gives coalesced memory access (consecutive threads read consecutive `re[i]`), though that was the secondary motivation behind keeping the arithmetic transparent.

*A bit-exact parity oracle before any benchmark.* Two identical registers, same gate, CPU path on one and CUDA path on the other, compared amplitude-by-amplitude. The result is `max difference = 0.000e+00` — not "within tolerance," but exactly zero, because both paths execute identical arithmetic in identical order on identical doubles. There is no cross-element reduction to reorder, so there's nothing to introduce a rounding difference. *Honest caveat:* exact zero is the *expected* result here given elementwise, reduction-free arithmetic — but it's an empirical observation, not a guarantee. A compiler that contracted `a*b + c` into a fused multiply-add on one path and not the other would introduce ~1e-16 differences that are non-associativity of floating point, not bugs, and would call for a tolerance instead of exact zero.

*The residency lifecycle, and why device pointers become members.* For state to persist across gate calls, the device memory must outlive any single method — so `d_re_` and `d_im_` are class *members*, not locals. `uploadToDevice()` mallocs them and copies up, once; `applyGateResident()` launches the kernel on the resident pointers and does nothing else — no malloc, no copy, no free; `downloadFromDevice()` syncs, copies down, frees, and resets. The pointer *variable* lives in host RAM; the data it addresses lives in device global memory. Between gates the host holds only the handle. The host scratch buffers (`re`/`im`) inside upload and download are per-method and disposable — they are *different variables* in each method, and the device members are what carry continuity.

*One `cudaDeviceSynchronize()` suffices for the whole circuit.* Kernel launches are asynchronous — the host issues a launch and races ahead. But gates issued to the same stream execute in issue order, so gate 2 sees gate 1's result without any intervening sync. `cudaDeviceSynchronize()` is a *host-side* barrier, needed only once, at download, to guarantee the GPU has finished before results are copied home. A per-gate sync would serialize host and device and throw away the entire residency benefit.

*The `on_device_` guard earns its keep.* An early bug placed `downloadFromDevice()` inside the gate loop, which freed the device memory and cleared the flag after the first gate; the second gate then hit the guard and threw. Without the guard, that same structural error would have launched a kernel on freed device pointers — undefined behavior producing silently wrong numbers in a benchmark that still "ran." The guard converts a silent correctness disaster into an immediate, located error. This is the general principle: defensive runtime checks are cheap insurance against the failure modes that don't announce themselves.

**Two hazards kept distinct.** A **race** is cross-thread: two threads touch the same location with no ordering, and the result depends on timing. A **read-after-write clobber** is single-thread: a value is overwritten before its last use. The gate kernel avoids the latter by snapshotting `ar, ai, br, bi` into locals *before* writing any output, so computing the new `state[j]` still sees the old `state[i]`. The CPU path does the same with `a` and `b`. Different bug, different fix (temporaries, not synchronization) — worth not conflating.

---

## Open questions and unverified claims

Kept explicit, because knowing the boundary between validated and assumed is part of the design:

- **Tunneling transmission coefficient vs. analytic WKB.** The Schrödinger solver reproduces tunneling qualitatively, but the numerical transmission coefficient `T` was never reconciled against the analytic WKB prediction. Treated as unverified.
- **The `k0` sign convention.** The wave-packet momentum parameter uses a sign convention that was never traced back to its source; it produces correct-looking dynamics but is not derived here.
- **The residency tail drift** (resident time rising 68 → 98 → 133 ms at large gate counts) is attributed to accumulating kernel-launch overhead. That is a reasonable inference from the roofline picture — the transfer tax is gone, so launches become the next bottleneck — but launch vs. residual transfer was not separately profiled. Likely explanation, not a measured decomposition.
- **The LU-once speedup** in Phase 3 is reasoned from complexity (O(n³) → O(n²)), not benchmarked.
- **CPU-vs-GPU absolute speedup** is deliberately absent everywhere: the CPU baseline was never re-measured for the identical single-gate operation at matched size, so any "N× vs CPU" figure would be unfounded. The transfer-bound ratios (transfer ≈ 23× compute, ≈ 96% of round-trip) use only GPU-side numbers and are safe.

---

For build and run instructions, see [SETUP.md](SETUP.md). For the project overview and the residency benchmark, see the [README](README.md).
