#include <iostream>
#include "../include/matrix.h"
#include "../include/vector.h"

int main() {
    using namespace quantum;

    // Solve Ax = b where:
    // | 2 1 1 |   |x|   |5|
    // | 1 3 2 | * |y| = |10|
    // | 1 0 0 |   |z|   |1|
    //
    // Expected solution: x=1, y=2, z=1

    Matrix<double> A(3, 3);
    A(0,0)=1; A(0,1)=0; A(0,2)=0;
    A(1,0)=0; A(1,1)=1; A(1,2)=0;
    A(2,0)=0; A(2,1)=0; A(2,2)=1;

    Vector<double> b(3);
    b(0) = 5;
    b(1) = 10;
    b(2) = 1;

    std::cout << "Matrix A:" << std::endl;
    A.print();

    std::cout << "Vector b: ";
    b.print();

    auto x = A.solve(b);

    std::cout << "Solution x: ";
    x.print();
    std::cout << "Expected: 1 2 1" << std::endl;

    // A simple 2x2: solve [[3,1],[1,2]] x = [5,4]
    // Expected: x = 1.2, y = 1.4
    Matrix<double> A2(2, 2);
    A2(0,0)=3; A2(0,1)=1;
    A2(1,0)=1; A2(1,1)=2;

    Vector<double> b2(2);
    b2(0) = 5;
    b2(1) = 4;

    auto x2 = A2.solve(b2);
    std::cout << "Solution: ";
    x2.print();
    std::cout << "Expected: 1.2 1.4" << std::endl;

    return 0;
}