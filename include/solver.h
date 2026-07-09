#ifndef SOLVER_H
#define SOLVER_H

#include <functional>
#include "vector.h"

/*      = 0 at the end of step — makes it a "pure virtual function." This makes Solver an abstract class — can't create a Solver directly, only derived classes that implement step().
        std::function<...> — a type-safe function wrapper. Lets one pass any callable (regular function, lambda, functor) as a parameter. ODEFunction<T> is a function that takes a time and state vector and returns the derivative.
        using ODEFunction<T> = ... — type alias for readability. Now we can write ODEFunction<double> instead of the long std::function signature.
*/

namespace quantum
{
    //f(t,y) returns dy/dt for system of ODEs
    template<typename T>
    using ODEFunction =  std :: function<Vector<T>(double, const Vector<T>&)>;


    template <typename T>
    class Solver
    {
        public:
            //Virtual Destructor: Needed when class is inherited so that when deleting a derived class through a base class pointer, right destructor runs
            virtual ~Solver() = default;

            //computes one step: takes current state y at time t, returns state at t + dt
            virtual Vector<T> step(double t, const Vector<T>& y, double dt, ODEFunction<T> f) = 0;
    };

}   

#endif