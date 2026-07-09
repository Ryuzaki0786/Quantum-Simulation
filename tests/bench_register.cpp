#include <iostream>
#include <chrono>
#include "../include/quantum_register.h"
#include "../include/quantum_gate.h"

int main() {
    using namespace quantum;
    int n = 24;                                    // 2^24 ≈ 16M amplitudes
    QuantumRegister reg(n);
    for (int q = 0; q < n; q++)
        reg.applyGateToQubit(gates::hadamard(), q); // uniform superposition

    auto start = std::chrono::high_resolution_clock::now();
    double p0 = 0.0;
    for (int rep = 0; rep < 50; rep++)
        p0 = reg.probabilityZero(0);
    auto end = std::chrono::high_resolution_clock::now();

    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "probabilityZero elapsed: " << ms << " ms\n";
    std::cout << "P(0) = " << p0 << " (expect 0.5)\n";
    return 0;
}