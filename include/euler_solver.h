#ifndef EULER_SOLVER_H
#define EULER_SOLVER_H

#include "solver.h"

namespace quantum {

    template <typename T>
    class EulerSolver: public Solver<T>
    {
        public:
            Vector<T> step(double t, const Vector<T>& y, double dt, ODEFunction<T> f) override{
                return (y + f(t,y) * dt);
            }
    };
}






#endif