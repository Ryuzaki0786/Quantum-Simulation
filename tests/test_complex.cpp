#include <iostream>
#include <complex>
#include "../include/matrix.h"
#include "../include/vector.h"

int main() {
    using namespace quantum;
    using cd = std::complex<double>;

    // Create a complex matrix
    Matrix<cd> A(2, 2);
    A(0,0) = cd(1, 0);  A(0,1) = cd(0, -1);   // 1, -i
    A(1,0) = cd(0, 1);  A(1,1) = cd(1, 0);    // i, 1

    std::cout << "Complex Matrix A:" << std::endl;
    A.print();

    // Multiply by itself
    auto B = A * A;
    std::cout << "A * A:" << std::endl;
    B.print();

    // Complex vector
    Vector<cd> v(2);
    v(0) = cd(1, 0);
    v(1) = cd(0, 1);   // 1, i

    std::cout << "Vector v: ";
    v.print();

    std::cout << "||v|| = " << v.norm() << " (expected sqrt(2) = 1.414)" << std::endl;

    return 0;
}