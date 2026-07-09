#ifndef QUANTUM_QUBIT_H
#define QUANTUM_QUBIT_H

#include <complex>
#include <random>
#include <stdexcept>
#include <cmath>
#include "matrix.h"   // pulls in vector.h; gives you Matrix<Complex> for gates
#include "vector.h"

namespace quantum {

    class Qubit {
    public:
        using Complex = std::complex<double>;

        // Default constructor: initialize to |0> = (1, 0) — the standard starting state
        Qubit();

        
        // Explicit constructor: caller supplies alpha, beta.
        // Should normalize (or at least check |alpha|^2 + |beta|^2 == 1).
        Qubit(Complex alpha, Complex beta);


        // --- derived quantities: COMPUTED, not stored ---
        double probability0() const;   // |alpha|^2  -> std::norm(alpha_)
        double probability1() const;   // |beta|^2   -> std::norm(beta_)

        // --- the core operation: apply a 2x2 unitary gate ---
        // new state = gate * [alpha, beta]^T
        void applyGate(const Matrix<Complex>& gate);

        // --- measurement: collapses the qubit ---
        // returns 0 or 1 with probabilities |alpha|^2, |beta|^2,
        // then sets the state to whichever outcome occurred.
        int measure();

        // --- debug ---
        void print() const;

        // accessors (handy for testing)
        Complex alpha() const { return state_(0); }
        Complex beta()  const { return state_(1); }

    private:
        Vector <Complex> state_;

        void normalize();   // private helper: rescale so |alpha|^2 + |beta|^2 = 1
    };
    Qubit  :: Qubit() : state_(2){
            state_(0) =  Complex(1.0,0.0); //alpha = 1
            state_(1) =  Complex(0.0,0.0); //Beta = 0
    }
    Qubit :: Qubit(Complex alpha, Complex beta) : state_(2){
            state_(0) =  alpha;
            state_(1) = beta;
            normalize();
    }

    double Qubit :: probability0() const{
        return std::norm(state_(0));
    }

    double Qubit :: probability1() const {
        return std::norm(state_(1));
    }

    void Qubit :: normalize(){
        state_ = state_.normalize();
    }

    void Qubit ::applyGate(const Matrix<Complex>& gate){
        state_ = gate * state_;
    }

    int Qubit :: measure (){
        static std :: random_device rd;
        static std :: mt19937 gen(rd());
        static std :: uniform_real_distribution<double> dist(0.0,1.0);

        double r = dist(gen);

        if (r < probability0()) {
            // outcome 0 — collapse to |0>
            state_(0) = Complex(1.0, 0.0);
            state_(1) = Complex(0.0, 0.0);
            return 0;
        } 
        else {
            // outcome 1 — collapse to |1>
            state_(0) = Complex(0.0, 0.0);
            state_(1) = Complex(1.0, 0.0);
            return 1;
        }

    }

} // namespace quantum
#endif