// tests/test_schrodinger.cpp
#include <fstream>
#include "../include/schrodinger.h"

int main() {
    // --- physical/grid parameters ---
    double length   = 1.0;          // domain [0, length]
    int    n_points = 500;          // grid resolution
    double mass     = 1.0;          // natural units for a first test
    double hbar     = 1.0;
    double dt       = 1e-5;         // SEE NOTE on dt below

    // --- wavepacket parameters ---
    double x0    = length / 4.0;    // start in left quarter, room to travel right
    double sigma = length / 20.0;   // narrow-ish but resolved
    double k0    = -500.0;           // +k0 propagates left, non-standard
    double barrier_center = 0.5;
    double barrier_width = 0.02;
    double V0 = 140000;

    quantum::SchrodingerEquation sim(length, n_points, mass, hbar, dt,
                                     x0, sigma, k0,barrier_center,barrier_width,V0);   // match YOUR ctor arg order

    std::ofstream out("schrodinger_output.csv");       // open ONCE (method #10)

    int n_steps = 1500;
    int output_every = 20;                             // ~100 snapshots, not 2000

    for (int s = 0; s < n_steps; ++s) {
        if (s % output_every == 0) sim.output(out);    // snapshot
        sim.step();                                    // advance
    }
    out.close();                                       // close ONCE

    return 0;
}