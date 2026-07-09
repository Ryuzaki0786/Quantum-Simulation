#include <iostream>
#include <fstream>
#include <cmath>
#include "../include/rk4_solver.h"

int main() {
    using namespace quantum;

    // Physical parameters
    double m = 1.0;   // mass
    double c = 0.2;   // damping coefficient
    double k = 1.0;   // spring constant

    // Define f(t, y) where y = [position, velocity]
    ODEFunction<double> f = [m, c, k](double t, const Vector<double>& y) {
        Vector<double> dydt(2);
        dydt(0) = y(1);                              // dx/dt = v
        dydt(1) = -(c/m) * y(1) - (k/m) * y(0);      // dv/dt
        return dydt;
    };

    RK4Solver<double> solver;

    // Initial conditions: position = 1, velocity = 0
    Vector<double> y(2);
    y(0) = 1.0;
    y(1) = 0.0;

    double dt = 0.01;
    double t = 0.0;
    double t_max = 30.0;

    // Output to CSV for plotting
    std::ofstream out("oscillator.csv");
    out << "t,position,velocity\n";

    while (t < t_max) {
        out << t << "," << y(0) << "," << y(1) << "\n";
        y = solver.step(t, y, dt, f);
        t += dt;
    }

    std::cout << "Simulation complete. Output written to oscillator.csv\n";
    std::cout << "Final position: " << y(0) << "\n";
    std::cout << "Final velocity: " << y(1) << "\n";

    return 0;
}