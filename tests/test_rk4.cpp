#include <iostream>
#include <cmath>
#include "../include/rk4_solver.h"

int main() {
    using namespace quantum;

    // ODE: dy/dt = -y, exact solution y(t) = e^(-t)
    // Define f(t, y) = -y
    ODEFunction<double> f = [](double t, const Vector<double>& y) {
        return y * (-1.0);
    };

    RK4Solver<double> solver;

    Vector<double> y(1);
    y(0) = 1.0;  // initial condition y(0) = 1

    double dt = 0.01;
    double t = 0.0;

    // Step until t = 1
    while (t < 1.0) {
        y = solver.step(t, y, dt, f);
        t += dt;
    }

    std::cout << "RK4 result at t=1: " << y(0) << std::endl;
    std::cout << "Exact (e^-1):        " << std::exp(-1.0) << std::endl;
    std::cout << "Error:               " << std::abs(y(0) - std::exp(-1.0)) << std::endl;

    return 0;
}