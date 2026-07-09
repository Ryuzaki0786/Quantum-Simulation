#include <iostream>
#include "../include/quantum_register.h"
#include "../include/quantum_gate.h"

int main() {
    using namespace quantum;

    // --- Test 1: H on qubit 0 of a 1-qubit register (simplest case) ---
    std::cout << "=== Test 1: H on q0, 1-qubit register ===\n";
    QuantumRegister reg1(1);
    reg1.applyGateToQubit(gates::hadamard(), 0);
    std::cout << "P(0)=" << reg1.probability(0)
              << "  P(1)=" << reg1.probability(1) << "\n";
    std::cout << "(expect 0.5, 0.5)\n\n";

    // --- Test 2: H on qubit 0 of a 2-qubit register (tests the pairing logic) ---
    std::cout << "=== Test 2: H on q0, 2-qubit register ===\n";
    QuantumRegister reg2(2);
    reg2.applyGateToQubit(gates::hadamard(), 0);
    for (int i = 0; i < 4; i++)
        std::cout << "P(" << i << ")=" << reg2.probability(i) << "  ";
    std::cout << "\n(expect P(0)=0.5, P(2)=0.5, others 0)\n\n";


    std::cout << "=== Test 3: H on q1, 2-qubit register ===\n";
    QuantumRegister reg3(2);
    reg3.applyGateToQubit(gates::hadamard(), 1);   // qubit 1 this time
    for (int i = 0; i < 4; i++)
        std::cout << "P(" << i << ")=" << reg3.probability(i) << "  ";
    std::cout << "\n(expect P(0)=0.5, P(2)=0.5, others 0)\n";


    std::cout << "=== Bell state: H(q0) then CNOT(0->1) ===\n";
    QuantumRegister bell(2);                      // |00> = [1,0,0,0]
    bell.applyGateToQubit(gates::hadamard(), 0);  // -> [1/√2, 0, 1/√2, 0]
    bell.applyCNOT(0, 1);                         // control=0, target=1 -> entangle
    for (int i = 0; i < 4; i++)
        std::cout << "P(" << i << ")=" << bell.probability(i) << "  ";
    std::cout << "\n(expect P(0)=0.5, P(3)=0.5, P(1)=P(2)=0)\n";

    std::cout << "=== DEBUG: Bell state before measuring ===\n";
    QuantumRegister dbg(2);
    dbg.applyGateToQubit(gates::hadamard(), 0);
    dbg.applyCNOT(0, 1);
    for (int i = 0; i < 4; i++)
        std::cout << "amp[" << i << "] = " << dbg.state()(i) << "\n";
    std::cout.flush();   // force it to print before any crash

    int out0 = dbg.measureQubit(0);
    std::cout << "measured qubit 0 -> " << out0 << "\n";
    std::cout.flush();
    for (int i = 0; i < 4; i++)
        std::cout << "after m0, amp[" << i << "] = " << dbg.state()(i) << "\n";
    std::cout.flush();

    std::cout << "=== measureQubit on Bell state ===\n";
    int matches = 0, trials = 1000;
    for (int trial = 0; trial < trials; ++trial) {
         QuantumRegister bell(2);
         bell.applyGateToQubit(gates::hadamard(), 0);
         bell.applyCNOT(0, 1);                 // Bell state: (|00> + |11>)/√2

         int m0 = bell.measureQubit(0);        // measure qubit 0
         int m1 = bell.measureQubit(1);        // then measure qubit 1
         if (m0 == m1) matches++;              // entanglement: they MUST agree
     }
     std::cout << "qubit0 == qubit1 in " << matches << "/" << trials << " trials\n";
     std::cout << "(expect 1000/1000 — perfect correlation = entanglement)\n";

     int zeros = 0;
     for (int trial = 0; trial < trials; ++trial) {
         QuantumRegister bell(2);
         bell.applyGateToQubit(gates::hadamard(), 0);
         bell.applyCNOT(0, 1);
         if (bell.measureQubit(0) == 0) zeros++;
     }
     std::cout << "qubit0 measured 0 in " << zeros << "/" << trials << " (expect ~500)\n";


    QuantumRegister qr(3);              // 3 qubits: 0=message, 1&2=Bell pair. Starts |000>

    // --- prepare the message on qubit 0 (test with |1>) ---
    qr.applyGateToQubit(gates::pauliX(), 0);   // qubit 0 now |1>  (the state to teleport)

    // --- Step 1: Bell pair between qubits 1 and 2 ---
    qr.applyGateToQubit(gates::hadamard(), 1);
    qr.applyCNOT(1, 2);

    // --- Step 2: entangle message with Alice's half ---
    qr.applyCNOT(0, 1);

    // --- Step 3: Hadamard on the message qubit ---
    qr.applyGateToQubit(gates::hadamard(), 0);

    // --- Step 4: Alice measures qubits 0 and 1 (two classical bits) ---
    int m0 = qr.measureQubit(0);
    int m1 = qr.measureQubit(1);

    // --- Step 5: Bob's conditional corrections on qubit 2 ---
    if (m1 == 1) qr.applyGateToQubit(gates::pauliX(), 2);
    if (m0 == 1) qr.applyGateToQubit(gates::pauliZ(), 2);

    // --- verify: qubit 2 should now be |1> ---
    std::cout << "measured bits: m0=" << m0 << " m1=" << m1 << "\n";
    int result = qr.measureQubit(2);
    std::cout << "teleported qubit 2 measured: " << result << " (expect 1)\n";


    std::cout << "=== Teleportation of |1>, 1000 trials ===\n";
    int correct = 0;
    int seen[2][2] = {{0,0},{0,0}};   // count which (m0,m1) combos occur
    for (int trial = 0; trial < 1000; ++trial) {
        QuantumRegister qr(3);
        qr.applyGateToQubit(gates::pauliX(), 0);        // message = |1>
        qr.applyGateToQubit(gates::hadamard(), 1);
        qr.applyCNOT(1, 2);
        qr.applyCNOT(0, 1);
        qr.applyGateToQubit(gates::hadamard(), 0);
        int m0 = qr.measureQubit(0);
        int m1 = qr.measureQubit(1);
        if (m1 == 1) qr.applyGateToQubit(gates::pauliX(), 2);
        if (m0 == 1) qr.applyGateToQubit(gates::pauliZ(), 2);
        seen[m0][m1]++;
        if (qr.measureQubit(2) == 1) correct++;
    }
    std::cout << "qubit 2 = 1 in " << correct << "/1000 (expect 1000)\n";
    std::cout << "(m0,m1) distribution: 00=" << seen[0][0] << " 01=" << seen[0][1]
            << " 10=" << seen[1][0] << " 11=" << seen[1][1] << "\n";
    std::cout << "(expect all four ~250 — proves all correction paths fire)\n";


    std::cout << "=== Teleportation phase test: H|0>, then H on qubit 2 ===\n";
    zeros = 0;
    for (int trial = 0; trial < 1000; ++trial) {
        QuantumRegister qr(3);
        qr.applyGateToQubit(gates::hadamard(), 0);      // message = (|0>+|1>)/√2
        qr.applyGateToQubit(gates::hadamard(), 1);
        qr.applyCNOT(1, 2);
        qr.applyCNOT(0, 1);
        qr.applyGateToQubit(gates::hadamard(), 0);
        int m0 = qr.measureQubit(0);
        int m1 = qr.measureQubit(1);
        if (m1 == 1) qr.applyGateToQubit(gates::pauliX(), 2);
        if (m0 == 1) qr.applyGateToQubit(gates::pauliZ(), 2);
        qr.applyGateToQubit(gates::hadamard(), 2);      // <-- the phase probe
        if (qr.measureQubit(2) == 0) zeros++;
    }
    std::cout << "qubit 2 = 0 in " << zeros << "/1000 (expect ~1000)\n";

    return 0;
}