#include <iostream>
#include "../include/qubit.h"
#include "../include/quantum_gate.h"

int main() {
    using namespace quantum;

    // Experiment 1: H|0> should give 50/50
    Qubit q;                          // starts in |0>
    q.applyGate(gates::hadamard());
    std::cout << "After H|0>:  P(0) = " << q.probability0()
              << ",  P(1) = " << q.probability1() << "\n";
    // expect 0.5, 0.5

    // Experiment 2: HH|0> should return to definite |0>  (the interference result)
    Qubit q2;
    q2.applyGate(gates::hadamard());
    q2.applyGate(gates::hadamard());
    std::cout << "After HH|0>: P(0) = " << q2.probability0()
              << ",  P(1) = " << q2.probability1() << "\n";
    // expect 1.0, 0.0  -- the destructive interference you derived by hand

    // Experiment 3: measure H|0> many times, count outcomes
    int count0 = 0, trials = 10000;
    for (int i = 0; i < trials; ++i) {
        Qubit qm;
        qm.applyGate(gates::hadamard());
        if (qm.measure() == 0) count0++;
    }
    std::cout << "Measured 0 in " << count0 << "/" << trials
              << " trials (expect ~5000)\n";

    return 0;
}