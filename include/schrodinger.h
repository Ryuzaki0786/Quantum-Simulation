#ifndef SCHRODINGER_H
#define SCHRODINGER_H

#include <complex>
#include <fstream>
#include <stdexcept>
#include "matrix.h"
#include "vector.h"


namespace quantum 
{
    class SchrodingerEquation{
        public:
           using Complex = std :: complex<double>;
        public:

           SchrodingerEquation(double length,int n_points,double mass,double hbar,double dt,double x0, double sigma, double k0,double barrier_center,double barrier_width,double V0 )
           :
            length_(length),
            n_points_(n_points),
            mass_(mass),
            hbar_(hbar),
            dt_(dt),
            x0_(x0),
            sigma_(sigma),
            k0_(k0),
            dx_(length / (n_points - 1)),
            barrier_center_(barrier_center),
            barrier_width_(barrier_width),
            V0_(V0),
            psi_(n_points),
            A_(n_points,n_points),
            B_(n_points,n_points),
            L_(n_points,n_points),
            U_(n_points,n_points)
            {
                if (n_points < 3) throw std::invalid_argument("need >= 3 points");
                if (dt <= 0.0 || length <= 0.0)
                    throw std::invalid_argument("dt, length must be positive");
                buildOperators();
                initializeWavepacket();
            }

            void step();
            const Vector<Complex>& state() const {return psi_;}
            double potential(double x) const;
            void output(std :: ofstream& out) const;

        private:
            double length_;
            int n_points_;
            double mass_;
            double hbar_;
            double dt_;
            double x0_;
            double sigma_;
            double k0_;
            double dx_;
            double barrier_center_;
            double barrier_width_;
            double V0_;

            Vector<Complex> psi_;
            Matrix<Complex> A_;
            Matrix<Complex> B_;
            Matrix<Complex> L_;
            Matrix<Complex> U_;

            void buildOperators();
            void initializeWavepacket();

    };

    void SchrodingerEquation :: buildOperators(){
        Matrix<Complex> H(n_points_,n_points_);

        const double diag    =  (hbar_ * hbar_) / (mass_ * dx_ * dx_);     // + V(x_i) later
        const double offdiag = -(hbar_ * hbar_) / (2.0 * mass_ * dx_ * dx_);

        for(int i = 1; i < n_points_ - 1; ++i)
        {
            double x = i * dx_;
            H(i,i) = diag + potential(x);
            H(i,i-1) = offdiag;
            H(i,i+1) = offdiag;
        }

        H(0,0) = 1.0;
        H(n_points_ - 1,n_points_ - 1) = 1.0;

        Matrix<Complex> I  = Matrix<Complex>:: identity(n_points_);
        Complex coeff (0.0,dt_ / (2.0 * hbar_));

        A_ = I + H * coeff;     // once, not in a loop
        B_ = I - H * coeff;

        int last = n_points_ - 1;                  // stamp identity boundary rows on A_, B_
        A_(0, 0) = 1.0;  A_(0, 1) = 0.0;
        B_(0, 0) = 1.0;  B_(0, 1) = 0.0;
        A_(last, last) = 1.0;  A_(last, last - 1) = 0.0;
        B_(last, last) = 1.0;  B_(last, last - 1) = 0.0;

        if (!A_.lu_decompose(L_, U_)) {
            throw std::runtime_error("Failed to factor Crank-Nicolson matrix A");
        }

    }

    void SchrodingerEquation :: step()
    {
        Vector<Complex> b = B_ * psi_;
        psi_ =  A_.solve_with_LU(L_,U_,b);
    }

    void SchrodingerEquation :: initializeWavepacket()
    {
        for(int i = 0; i < n_points_ - 1; i++)
        {
            double x = i * dx_;
            double envelope = std::exp(-(x - x0_) * (x - x0_) / (2.0 * sigma_ * sigma_));
            Complex phase = std::exp(Complex(0.0, k0_ * x));   // e^{i k0 x}
            psi_(i) = envelope * phase; 
        }
        psi_.normalize();
    }
    double SchrodingerEquation::potential(double x) const {
        double left  = barrier_center_ - barrier_width_ / 2.0;
        double right = barrier_center_ + barrier_width_ / 2.0;
        if (x >= left && x < right) {
            return V0_;
        }
        return 0.0;
    }

    void SchrodingerEquation::output(std::ofstream& out) const {
        for (int i = 0; i < n_points_; ++i) {
            out << std::norm(psi_(i));        // |psi|^2  (squared magnitude, real double)
            if (i < n_points_ - 1) out << ",";   // comma between, not after last
        }
        out << "\n";
    }


}



#endif