#ifndef QUANTUM_REGISTER_H
#define QUANTUM_REGISTER_H


#include <complex>
#include <stdexcept>
#include <cmath>
#include <random>
#include "matrix.h"
#include "vector.h"
#include <chrono>
#include <omp.h>

    #ifdef __CUDACC__
    #include <cuda_runtime.h>

    __global__ void applyGateKernel(double* re, double* im, int size, int bit,
                                    double g00r, double g00i, double g01r, double g01i,
                                    double g10r, double g10i, double g11r, double g11i) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx >= size) return;
        if ((idx & bit) != 0) return;

        int j = idx | bit;

        double ar = re[idx], ai = im[idx];
        double br = re[j],   bi = im[j];

        re[idx] = (g00r*ar - g00i*ai) + (g01r*br - g01i*bi);
        im[idx] = (g00r*ai + g00i*ar) + (g01r*bi + g01i*br);

        re[j]   = (g10r*ar - g10i*ai) + (g11r*br - g11i*bi);
        im[j]   = (g10r*ai + g10i*ar) + (g11r*bi + g11i*br);
    }
    #endif

namespace quantum {
    class QuantumRegister {
        public:
            using Complex = std :: complex<double>;

            explicit QuantumRegister(int n);

            int numQubits() const {return n_qubits_;}
            int size() const { return state_.size(); }

            double probability(int basis_state) const;

            void applyGate(const Matrix<Complex>& gate);

            void applyGateToQubit(const Matrix<Complex>& gate,int target);

            void applyGateToQubitCUDA(const Matrix<Complex>& gate, int target);

            void applyCNOT(int control,int target);

            int measureQubit (int target);

            void print() const;

            void uploadToDevice();                                    // malloc + copy-up, once
            void applyGateResident(const Matrix<Complex>& gate, int target);  // launch only
            void downloadFromDevice();                                // copy-down + free, once

            double probabilityZero(int target) const;

            const Vector<Complex>& state() const {return state_; }
        

        private:
            int n_qubits_;
            Vector<Complex> state_;

            double* d_re_ = nullptr; // resident device real parts
            double* d_im_ = nullptr; //resident device imag parts
            bool on_device = false; //is the state currently resident on the GPU ?

    };

    QuantumRegister :: QuantumRegister(int n)
    : n_qubits_(n),
        state_(1 << n)
        {
            if( n < 1) throw std::invalid_argument("QuantumRegister needs at least 1 quibit");
            state_(0) = Complex(1.0,0.0);
        }
    double QuantumRegister :: probability(int basis_state) const
    {
        return (std :: norm(state_(basis_state)));
    }

    void QuantumRegister :: applyGate(const Matrix<Complex>& gate)
    {
        int dim = size();
        if(gate.rows() != dim || gate.cols() != dim){
            throw std::invalid_argument("Whole-register gate must be 2^n x 2^n");
        }
        state_ = gate * state_;
    }

    void QuantumRegister :: applyGateToQubit(const Matrix<Complex>& gate, int target){
        if (target < 0 || target > n_qubits_ - 1)
            throw std::invalid_argument("target qubit out of range");
        if (gate.rows() != 2 || gate.cols() != 2)
            throw std::invalid_argument("single-qubit gate must be 2x2");

        int bit = 1 << target;
        #pragma omp parallel for          
        for(int i = 0; i < size();i++)
        {
            if((i & bit) == 0){
                int j = i | bit;
                Complex a = state_(i);
                Complex b = state_(j);

                state_(i) = gate(0,0) * a + gate(0,1) * b;
                state_(j) = gate(1,0) * a + gate(1,1) * b;
                
            }
        }
    }

    void QuantumRegister :: applyCNOT(int control, int target){

        if (control < 0 || control >= n_qubits_ || target < 0 || target >= n_qubits_)
            throw std::invalid_argument("CNOT qubit index out of range");
        if (control == target)
            throw std::invalid_argument("CNOT control and target must differ");

        int c = 1 << control;
        int t = 1 << target;

        for(int i = 0; i < size(); i++)
        {
            if((i & c) != 0 && (i & t) == 0){
                int j = i | t;

                Complex temp = state_(i);
                state_(i) = state_(j);
                state_(j) = temp;
            }
        }
    }

    int QuantumRegister :: measureQubit(int target){
        int t = 1 << target;

        double p0 = 0.0;
        #pragma omp parallel for reduction(+:p0)
        //P(0) = sum of |amplitude|^2 where target bit is 0
        for(int i = 0; i < size(); i++)
        {
            if((i & t) == 0) p0 += std::norm(state_(i));
        }

        //random draw
        static std:: random_device rd;
        static std:: mt19937 gen(rd());
        static std :: uniform_real_distribution<double> dist(0.0,1.0);
        double r = dist(gen);
        int outcome = (r < p0) ? 0 : 1;

        //Collapse - zero out amplitudes inconsistent with the outcome
        #pragma omp parallel for
        for(int i = 0; i < size(); i++)
        {
            int bit_value = ((i & t) != 0) ? 1 : 0;   // target qubit's value in basis state i
            if (bit_value != outcome) {
                state_(i) = Complex(0.0, 0.0);        // inconsistent → zero it
            }
        }

        state_ = state_.normalize();

        return outcome;
    }
    double QuantumRegister::probabilityZero(int target) const {
        int t = 1 << target;
        double p0 = 0.0;
        #pragma omp parallel for reduction(+:p0)
        for (int i = 0; i < size(); i++) {
            if ((i & t) == 0) p0 += std::norm(state_(i));
        }
        return p0;
    }

        #ifdef __CUDACC__
        void QuantumRegister::applyGateToQubitCUDA(const Matrix<Complex>& gate, int target) {
            if (target < 0 || target > n_qubits_ - 1)
                throw std::invalid_argument("target qubit out of range");
            if (gate.rows() != 2 || gate.cols() != 2)
                throw std::invalid_argument("single-qubit gate must be 2x2");

            int N   = size();
            int bit = 1 << target;

            // boundary 1: layout (AoS std::complex -> SoA split doubles), host side
            std::vector<double> re(N), im(N);
            for (int i = 0; i < N; i++) {
                re[i] = state_(i).real();
                im[i] = state_(i).imag();
            }

            double g00r = gate(0,0).real(), g00i = gate(0,0).imag();
            double g01r = gate(0,1).real(), g01i = gate(0,1).imag();
            double g10r = gate(1,0).real(), g10i = gate(1,0).imag();
            double g11r = gate(1,1).real(), g11i = gate(1,1).imag();

            // boundary 2: memory space (host RAM <-> device global memory)
            double *d_re, *d_im;
            size_t bytes = static_cast<size_t>(N) * sizeof(double);
            cudaMalloc(&d_re, bytes);
            cudaMalloc(&d_im, bytes);
            cudaMemcpy(d_re, re.data(), bytes, cudaMemcpyHostToDevice);
            cudaMemcpy(d_im, im.data(), bytes, cudaMemcpyHostToDevice);

            int threads = 256;
            int blocks  = (N + threads - 1) / threads;
            applyGateKernel<<<blocks, threads>>>(d_re, d_im, N, bit,
                g00r, g00i, g01r, g01i, g10r, g10i, g11r, g11i);
            cudaDeviceSynchronize();

            cudaMemcpy(re.data(), d_re, bytes, cudaMemcpyDeviceToHost);
            cudaMemcpy(im.data(), d_im, bytes, cudaMemcpyDeviceToHost);
            cudaFree(d_re);
            cudaFree(d_im);

            // boundary 1 reversed: SoA -> AoS, write results back into the class
            for (int i = 0; i < N; i++)
                state_(i) = Complex(re[i], im[i]);
        }
    void QuantumRegister :: uploadToDevice(){
        int N = size();

        std::vector<double> re(N), im(N);
        for(int i = 0; i < N; i++){
            re[i] = state_(i).real();
            im[i] = state_(i).imag();
        }

        size_t bytes =  static_cast<size_t>(N) * sizeof(double);
        cudaMalloc(&d_re_,bytes);
        cudaMalloc(&d_im_,bytes);

        cudaMemcpy(d_re_, re.data(), bytes, cudaMemcpyHostToDevice);
        cudaMemcpy(d_im_, im.data(), bytes, cudaMemcpyHostToDevice);

        on_device_ = true;

    }

    void QuantumRegister :: applyGateResident(const Matrix<Complex>& gate, int target){
        if(!on_device_){
            throw std::runtime_error("must call uploadToDevice() before applyGateResident()");
        }

        int N = size();
        int bit = 1 << target;

        double g00r = gate(0,0).real(), g00i = gate(0,0).imag();
        double g01r = gate(0,1).real(), g01i = gate(0,1).imag();
        double g10r = gate(1,0).real(), g10i = gate(1,0).imag();
        double g11r = gate(1,1).real(), g11i = gate(1,1).imag();

        int threads = 256;
        int blocks = (N + threads - 1) / threads;

        applyGateKernel<<<blocks, threads>>>(d_re_, d_im_, N, bit,
                g00r, g00i, g01r, g01i, g10r, g10i, g11r, g11i);

    }

    void QuantumRegister:: downloadFromDevice(){
        int N = size();
        std::vector<double> re(N), im(N);

        cudaDeviceSynchronize();

        size_t bytes =  static_cast<size_t>(N) * sizeof(double);
        cudaMemcpy(re.data(), d_re_, bytes, cudaMemcpyDeviceToHost);
        cudaMemcpy(im.data(), d_im_, bytes, cudaMemcpyDeviceToHost);

        for (int i = 0; i < N; i++)
            state_(i) = Complex(re[i], im[i]);

        cudaFree(d_re_);
        cudaFree(d_im_);
        d_re_ = nullptr;
        d_im_ = nullptr;
        on_device_ = false;
    }
    #endif

    

}

#endif