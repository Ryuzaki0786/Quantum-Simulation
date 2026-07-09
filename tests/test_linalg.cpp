#define CATCH_CONFIG_MAIN     // generates main() — exactly ONE file does this
#include "lib/catch.hpp"
#include "../include/vector.h"
#include "../include/matrix.h"

#include <stdexcept>

using namespace quantum;

TEST_CASE("Vector dot product", "[vector]") {
    // Arrange
    Vector<double> a(3);
    a(0) = 1.0; a(1) = 2.0; a(2) = 3.0;

    Vector<double> b(3);
    b(0) = 4.0; b(1) = 5.0; b(2) = 6.0;

    // Act
    double result = a.dot(b);

    // Assert
    REQUIRE(result == Approx(32.0));   // 1*4 + 2*5 + 3*6 = 32
}

TEST_CASE("Vector Norm","[vector]")
{
    // Arrange
    Vector<double> a(2);
    a(0) = 3.0; a(1) = 4.0;

    // Act
    double result = a.norm();

    //Assert
    REQUIRE(result == Approx(5.0));
}

TEST_CASE("Vector exception", "[vector]") {
    // Arrange
    Vector<double> a(3);
    a(0) = 1.0; a(1) = 2.0; a(2) = 3.0;

    Vector<double> b(4);
    b(0) = 4.0; b(1) = 5.0; b(2) = 6.0; b(3) = 7.0;

    // Assert
    REQUIRE_THROWS_AS(a + b, std::invalid_argument);
}

TEST_CASE("Identity matrix multiplication", "[matrix]") {
    Matrix<double> A(2, 2);
    A(0,0) = 1.0; A(0,1) = 2.0;
    A(1,0) = 3.0; A(1,1) = 4.0;

    Matrix<double> I = Matrix<double>::identity(2);
    Matrix<double> result = I * A;

    REQUIRE(result(0,0) == Approx(1.0));
    REQUIRE(result(0,1) == Approx(2.0));
    REQUIRE(result(1,0) == Approx(3.0));
    REQUIRE(result(1,1) == Approx(4.0));
}

TEST_CASE("Dimension Mismatch execption", "[matrix]") {
    Matrix<double> A(2, 2);
    A(0,0) = 1.0; A(0,1) = 2.0;
    A(1,0) = 3.0; A(1,1) = 4.0;

    Matrix<double> B(3, 3);
    B(0,0) = 1.0; B(0,1) = 2.0; B(0,2) = 2.5;
    B(1,0) = 3.0; B(1,1) = 4.0; B(1,2) = 4.5;
    B(2,0) = 3.5; B(2,1) = 4.2; B(2,2) = 4.7;

    REQUIRE_THROWS_AS(A * B, std::invalid_argument);
}
