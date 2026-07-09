#ifndef WAVE_EQUATION_H
#define WAVE_EQUATION_H


#include "vector.h"
#include <fstream>
#include <cmath>
#include <stdexcept>

namespace quantum{
    
    class WaveEquation{
        public:
            WaveEquation(double length, int n_points, double c,double dt)
            :   n_points_(n_points),
                dx_(length/(n_points - 1)),
                c_(c),
                dt_(dt),
                r_(c * dt / dx_),
                u_prev_(n_points),
                u_curr_(n_points),
                u_next_(n_points)
            {  
                if(r_ > 1.0) {
                    throw std::invalid_argument("CFL condition violated: c * dt / dx must be <= 1");
                }
            }

            void initialize();
            void step();
            void output(std::ofstream &);
        private:
            //Grid Parameters
            int n_points_;
            double dx_;
            double c_;
            double dt_;
            double r_;

            Vector<double> u_prev_;
            Vector<double> u_curr_;
            Vector<double> u_next_;
    };

    void WaveEquation :: initialize()
    {
        double x0 = (n_points_ - 1) * dx_ / 2.0;  //center of the string
        double sigma = (n_points_ - 1) * dx_ / 20.0;  //Pulse width

        for(int i = 0; i < n_points_; i++){
            double x =  i * dx_;
            u_curr_(i) =  std::exp ( -((x - x0) * (x - x0)) / (2.0 * sigma * sigma));

        }
        u_prev_ = u_curr_;
    }
    
    void WaveEquation :: step()
    {
        
        for(int i = 1; i <= n_points_ - 2; i++)
        {
            u_next_(i) = 2 * u_curr_(i) - u_prev_(i) + (r_ * r_) * (u_curr_( i + 1) - (2 * u_curr_(i)) + u_curr_(i - 1));
        }
        u_next_(0) = 0;
        u_next_(n_points_ - 1) = 0;

        u_prev_ = u_curr_;
        u_curr_ = u_next_;
    }

    void WaveEquation :: output(std::ofstream &file){
        for(int i = 0; i < n_points_;i++){
            file << u_curr_(i);
            if(i < n_points_ - 1){
                file << ",";
            }
        }
        file << "\n";
    }
}

#endif