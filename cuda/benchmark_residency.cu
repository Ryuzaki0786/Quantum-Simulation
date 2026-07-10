#include "../include/quantum_register.h"
#include "../include/matrix.h"
#include <cstdio>
#include <cmath>
#include <vector>

using namespace quantum;
using Complex = std::complex<double>;

int main() {
    int n = 20;                 // 20 qubits = ~1M amplitudes, realistic size
    int target_count = n;

    // Hadamard gate
    double s = 1.0 / std::sqrt(2.0);
    Matrix<Complex> H(2, 2);
    H(0,0) = Complex(s,0);  H(0,1) = Complex(s,0);
    H(1,0) = Complex(s,0);  H(1,1) = Complex(-s,0);

    // sweep over circuit lengths (number of gates)
    std::vector<int> gate_counts = {1, 2, 4, 8, 16, 32, 64, 128, 256};

    // --- warm-up: first CUDA call pays one-time context init, discard it ---
    {
        QuantumRegister warm(n);
        warm.applyGateToQubitCUDA(H, 0);   // triggers context creation
    }

    printf("%8s %16s %16s %10s\n", "gates", "per-gate (ms)", "resident (ms)", "speedup");

    for (int N : gate_counts) {
        cudaEvent_t start, stop;
        cudaEventCreate(&start);
        cudaEventCreate(&stop);
        float ms_pergate = 0.0f, ms_resident = 0.0f;

        // ---------- PER-GATE (naive): full round-trip every gate ----------
        {
            QuantumRegister reg(n);
            cudaEventRecord(start);

            // TODO: loop N times, each iteration calls applyGateToQubitCUDA
            //       with target = (k % target_count)
            for(int k = 0; k < N; k++)
            {
                reg.applyGateToQubitCUDA(H, k % target_count);
            }

            cudaEventRecord(stop);
            cudaEventSynchronize(stop);
            cudaEventElapsedTime(&ms_pergate, start, stop);
        }

        // ---------- RESIDENT: upload once, N launches, download once ----------
        {
            QuantumRegister reg(n);
            cudaEventRecord(start);
            reg.uploadToDevice();
            // TODO: uploadToDevice()
            //       loop N times: applyGateResident(H, k % target_count)
            //       downloadFromDevice()
            for(int k = 0; k < N; k++){
                reg.applyGateResident(H, k % target_count);
                reg.downloadFromDevice();
            }

            cudaEventRecord(stop);
            cudaEventSynchronize(stop);
            cudaEventElapsedTime(&ms_resident, start, stop);
        }

        double speedup = ms_pergate / ms_resident;
        printf("%8d %16.3f %16.3f %9.2fx\n", N, ms_pergate, ms_resident, speedup);

        cudaEventDestroy(start);
        cudaEventDestroy(stop);
    }

    return 0;
}