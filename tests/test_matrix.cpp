#include <iostream>
#include "../include/matrix.h"

int main() {
    quantum::Matrix<double> A(2, 2);
    A(0,0) = 1; A(0,1) = 2;
    A(1,0) = 3; A(1,1) = 4;

    quantum::Matrix<double> B(2, 2);
    B(0,0) = 5; B(0,1) = 6;
    B(1,0) = 7; B(1,1) = 8;

    std::cout << "A:" << std::endl;
    A.print();

    std::cout << "B:" << std::endl;
    B.print();

    auto C = A + B;
    std::cout << "A + B:" << std::endl;
    C.print();

    auto D = A * B;
    std::cout << "A * B:" << std::endl;
    D.print();

    auto I = quantum::Matrix<double>::identity(3);
    std::cout << "Identity 3x3:" << std::endl;
    I.print();

    auto At = A.transpose();
    std::cout << "A transposed:" << std::endl;
    At.print();
    
    return 0;
}