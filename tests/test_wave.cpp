#include "../include/wave_equation.h"
#include <fstream>

int main()
{
    quantum::WaveEquation wave(1.0, 200, 1.0, 0.004);  // length=1, 200 points, c=1, dt=0.004
    wave.initialize();

    std::ofstream file("wave.csv");   // open ONCE here

    for (int n = 0; n < 500; n++) {   // 500 timesteps
        wave.output(file);            // write current state
        wave.step();                  // advance one timestep
    }

    file.close();                     // close ONCE here
    return 0;
}