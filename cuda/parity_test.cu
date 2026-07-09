#include "../quantum_register.h"
#include "../matrix.h"
#include <cstdio>
#include <cmath>

using namespace quantum;
using Complex = std::complex<double>;

int main() {
    int n = 12;                 // 12 qubits = 4096 amplitudes, plenty for a parity check
    int target = 0;

    // Hadamard gate: (1/sqrt2) [[1,1],[1,-1]]
    double s = 1.0 / std::sqrt(2.0);
    Matrix<Complex> H(2, 2);
    H(0,0) = Complex(s,0);  H(0,1) = Complex(s,0);
    H(1,0) = Complex(s,0);  H(1,1) = Complex(-s,0);

    // two identical registers
    QuantumRegister cpuReg(n);
    QuantumRegister gpuReg(n);

    // put them in a non-trivial state first so the test isn't trivial:
    // spread amplitude with a Hadamard on qubit 1 (CPU path, same for both)
    Matrix<Complex> Hsetup = H;
    cpuReg.applyGateToQubit(Hsetup, 1);
    gpuReg.applyGateToQubit(Hsetup, 1);

    // now the actual thing under test: same gate, two different backends
    cpuReg.applyGateToQubit(H, target);      // CPU
    gpuReg.applyGateToQubitCUDA(H, target);  // GPU

    // compare every amplitude
    const Vector<Complex>& cs = cpuReg.state();
    const Vector<Complex>& gs = gpuReg.state();

    double maxDiff = 0.0;
    for (int i = 0; i < cpuReg.size(); i++) {
        double d = std::abs(cs(i) - gs(i));
        if (d > maxDiff) maxDiff = d;
    }

    printf("qubits = %d, amplitudes = %d\n", n, cpuReg.size());
    printf("max amplitude difference (CPU vs CUDA) = %.3e\n", maxDiff);
    printf(maxDiff < 1e-12 ? "PARITY PASS\n" : "PARITY FAIL\n");

    return 0;
}