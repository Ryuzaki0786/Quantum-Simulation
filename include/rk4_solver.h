#ifndef RK4_SOLVER_H
#define RK4_SOLVER_H

#include "solver.h"

namespace quantum {
    template <typename T>
    class RK4Solver : public Solver<T> {
        public:
            Vector<T> step(double t, const Vector<T>& y, double dt, ODEFunction<T> f) override{
                Vector<T> k1 = f(t,y);                            //Slope at start
                Vector<T> k2 = f(t + dt/2, y + k1 * (dt / 2));         //slope at midpoint using k1
                Vector<T> k3 = f(t + dt/2, y + k2 * (dt / 2));        // slope at midpoint using k2
                Vector<T> k4 = f(t + dt, y + k3 * dt);            // slope at the end using k3

                Vector<T> y_next = y + (k1 + k2*2.0 + k3*2.0 + k4) * (dt / 6);

                return y_next;
            }
    };
}


#endif