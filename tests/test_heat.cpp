#include "../include/heat_equation.h"
#include <fstream>
#include <chrono>
#include <omp.h>


int main()
{
    // dx = 1.0/99 ≈ 0.0101,  dx² ≈ 0.000102
    // r = α·dt/dx² = 1.0 · 0.00004 / 0.000102 ≈ 0.39  (≤ 0.5 ✓)
    quantum::HeatEquation heat(1.0, 100, 1.0, 0.00004);
    // double target_r = 0.4;
    // int N = 2000000;
    // double dx = 1.0 / (N - 1);
    // double dt = target_r * dx * dx / 1.0;   // alpha = 1.0
    // quantum::HeatEquation heat(1.0, N, 1.0, dt);
    heat.initialize();

    std::cout << "Threads available: " << omp_get_max_threads() << "\n";

    auto start = std::chrono::high_resolution_clock::now();
    std::ofstream file("heat.csv");
    for (int n = 0; n < 20; n++) {
        if (n % 20 == 0) {       // output every 20th step → 200 rows, manageable
             heat.output(file);
         }
        heat.step();
    }
    auto end = std ::chrono::high_resolution_clock::now();
    double ms =  std :: chrono :: duration<double,std::milli>(end - start).count();
    std :: cout <<"Elapsed: " << ms << " ms\n";
    file.close();
    return 0;
}