#include <iostream>
#include <cmath>
#include "../include/vector.h"

int main() {
    using namespace quantum;

    // Create two vectors
    Vector<double> a(3);
    a(0) = 1; a(1) = 2; a(2) = 3;

    Vector<double> b(3);
    b(0) = 4; b(1) = 5; b(2) = 6;

    std::cout << "a: ";
    a.print();
    std::cout << "b: ";
    b.print();

    // Addition
    auto c = a + b;
    std::cout << "a + b: ";
    c.print();

    // Subtraction
    auto d = b - a;
    std::cout << "b - a: ";
    d.print();

    // Scalar multiplication
    auto e = a * 2.0;
    std::cout << "a * 2: ";
    e.print();

    // Dot product
    double dot = a.dot(b);
    std::cout << "a . b = " << dot << " (expected 32)" << std::endl;

    // Norm
    double n = a.norm();
    std::cout << "||a|| = " << n << " (expected " << std::sqrt(14.0) << ")" << std::endl;

    // Normalize
    auto u = a.normalize();
    std::cout << "a normalized: ";
    u.print();
    std::cout << "||normalized a|| = " << u.norm() << " (expected 1.0)" << std::endl;

    return 0;
}