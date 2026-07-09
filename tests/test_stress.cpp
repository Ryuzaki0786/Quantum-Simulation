#include <iostream>
#include <chrono>
#include "../include/matrix.h"

int main() {
    using namespace quantum;
    using namespace std::chrono;

    const int N = 200;

    Matrix<double> A(N, N);
    Matrix<double> B(N, N);

    // Fill with simple pattern
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A(i, j) = (i + j) * 0.5;
            B(i, j) = (i - j) * 0.3;
        }
    }

    auto start = high_resolution_clock::now();
    auto C = A * B;
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);
    std::cout << N << "x" << N << " matrix multiplication: "
              << duration.count() << " ms" << std::endl;

    // Verify bounds checking works
    try {
        double x = A(N, N);  // out of bounds
        std::cout << "FAIL: should have thrown" << std::endl;
    } catch (std::out_of_range& e) {
        std::cout << "Bounds check working: " << e.what() << std::endl;
    }

    return 0;
}