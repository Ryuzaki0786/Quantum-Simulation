#ifndef HEAT_EQUATION_H
#define HEAT_EQUATION_H


#include "vector.h"
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <chrono>
#include <omp.h>

namespace quantum {
    class HeatEquation {
        public:
            HeatEquation(double length,int n_points, double alpha,double dt)
            :   n_points_(n_points),
                dx_(length/(n_points - 1)),
                alpha_(alpha),
                dt_(dt),
                r_(alpha * dt / (dx_ * dx_)),
                u_curr_(n_points),
                u_next_(n_points)
            {
                if(r_ > 0.5){
                    throw std::invalid_argument("Stability violated: alpha*dt/dx^2 must be <= 0.5");
                }
            }

            void initialize();
            void step();
            void output(std::ofstream&);

        private:
            int n_points_;
            double dx_;
            double alpha_;
            double dt_;
            double r_;

            Vector<double>u_curr_;
            Vector<double>u_next_;
    };

    void HeatEquation :: initialize()
    {
        for(int i = 0; i < n_points_; i++)
        {
            u_curr_(i) = (i < n_points_ / 2) ? 1.0 : 0.0;
        }
    }

    void HeatEquation :: step()
    {
        #pragma omp parallel for
        for(int i  = 1; i <= n_points_ - 2; i++)
        {
            u_next_(i) = u_curr_(i) + r_ * (u_curr_(i+1) - 2*u_curr_(i) + u_curr_(i-1));
        }
        u_next_(0) = 0;
        u_next_(n_points_ - 1) = 0;

        //u_curr_ = u_next_;
        std::swap(u_curr_,u_next_);
    }

    void HeatEquation :: output(std::ofstream &file)
    {
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