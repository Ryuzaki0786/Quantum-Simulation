#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <stdexcept>
#include <complex>

namespace quantum {
    template <typename T> class Vector;  // forward declaration
}

namespace quantum
{
    template <typename T>

    class Matrix
    {
        private:
            int rows_;
            int cols_;

            //store a 2D matrix as a 1D array. Element (i, j) lives at index i * cols_ + j. This is row-major order — same row elements are next to each other in memory, which is cache-friendly.
            std :: vector<T> data_;
        public:
            //Constructors
            Matrix(int rows,int cols);
            Matrix(int rows,int cols, T initial_value);

            //Element access - mat(i,j)
            T& operator() (int i,int j);
            const T& operator()(int i, int j) const;

            //Getters
            int rows() const {return rows_;}
            int cols() const {return cols_;}

            //Matrix operations
            Matrix<T> operator+(const Matrix<T>& other) const;
            Matrix<T> operator-(const Matrix<T>& other) const;
            Matrix<T> operator*(const Matrix<T>& other) const;
            Matrix<T> operator*(const T& s) const;
            Vector<T> operator*(const Vector<T>& v) const;
            Vector<T> solve_with_LU(const Matrix<T>& L, const Matrix<T>& U, const Vector<T>& b);

            //LU decomposition : L and U are output Parameters
            bool lu_decompose(Matrix<T>& L, Matrix<T>& U) const;
            Vector<T> solve(const Vector<T>& b) const;

            //Utility
            static Matrix<T> identity(int n);
            Matrix<T> transpose() const;
            void print() const;
    };

    // Creates a rows x cols matrix with every element zero-initialized (T(0)).
    // The flat vector is sized rows*cols; element (i,j) maps to index i*cols+j.
    template <typename T>
    Matrix<T> :: Matrix(int rows, int cols)
        : rows_(rows), cols_(cols),data_(rows * cols, T(0)) {}

    // Creates a rows x cols matrix with every element set to initial_value.
    template <typename T>
    Matrix<T> :: Matrix(int rows, int cols, T initial_value)
        : rows_(rows), cols_(cols), data_(rows * cols, initial_value) {}
    
    template <typename T>
    T& Matrix<T> ::operator()(int i, int j)
    {
        if (i < 0 || i >= rows_ || j < 0 || j >= cols_) {
            throw std::out_of_range("Matrix index out of bounds");
        }
        return data_[i * cols_ + j ];
    }
    template <typename T>
    const T& Matrix<T>::operator()(int i, int j) const {
        if (i < 0 || i >= rows_ || j < 0 || j >= cols_) {
            throw std::out_of_range("Matrix index out of bounds");
        }
        return data_[i * cols_ + j];
    }

    template <typename T>
    Matrix<T> Matrix<T> :: operator+(const Matrix<T>& other) const{
        if(rows_ != other.rows_ || cols_ != other.cols_)
        {
            throw std::invalid_argument("Matrix dimensions do not match for addition");
        }
        Matrix<T> result(rows_,cols_);
        for(int i = 0; i < rows_ * cols_;i++)
        {
            result.data_[i] = data_[i] + other.data_[i];
        }
        return result;
    }
    template <typename T>
    Matrix<T> Matrix<T> :: operator-(const Matrix<T>& other) const{
        if(rows_ != other.rows_ || cols_ != other.cols_)
        {
            throw std::invalid_argument("Matrix dimensions do not match for subtraction");
        }
        Matrix<T> result(rows_,cols_);
        for(int i = 0; i < rows_ * cols_;i++)
        {
            result.data_[i] = data_[i] - other.data_[i];
        }
        return result;
    }

    template <typename T>
    Matrix<T> Matrix<T> :: operator*(const Matrix<T>& other) const{
        if(cols_ != other.rows_)
        {
            throw std::invalid_argument("Matrix dimensions do not match for Multiplication");
        }
        Matrix<T> result(rows_,other.cols_);
        for(int i = 0; i<rows_;i++)
        {
            for(int j = 0;j < other.cols_; j++)
            {
                for(int k = 0; k < cols_; k++)
                {
                    result.data_[i * other.cols_ + j] += data_[i * cols_ + k] * other.data_[k * other.cols_ + j];
                }
            }
        }
        return result;
    }

    template <typename T>
    Matrix<T> Matrix<T> :: transpose() const {
        Matrix<T> result(cols_,rows_); //flipped dimensions

        for(int i = 0;i < rows_;i++)
        {
            for(int j = 0; j <cols_;j++)
            {
                result.data_[j * rows_ + i] = data_[i * cols_ + j];
            }
        }
        return result;
    }

    template <typename T>
    Matrix<T> Matrix<T>::identity(int n){
        Matrix<T> result(n,n);
        for(int i = 0 ; i < n;i++)
        {
            result.data_[i * n + i] = 1;
        }
        return result;
    }

    template <typename T>
    void Matrix<T>::print() const{
        for(int i = 0; i < rows_;i++)
        {
            for(int j = 0; j<cols_;j++)
            {
                std::cout <<data_[i * cols_ + j]<< " ";
            }
            std::cout<<"\n";
        }
    }

    template <typename T>
    bool Matrix<T> :: lu_decompose(Matrix<T>& L, Matrix<T>& U) const{
        if(rows_ != cols_)
        {
            throw std::invalid_argument("LU decomposition requires a square matrix");
        }

        int n = rows_;
        L = Matrix<T> (n,n);

        for(int i = 0; i<n; i++)
        {
            //Upper Triangular - U row i
            for(int k = i; k < n; k++)
            {
                T sum = T(0);
                for(int j = 0; j < i; j++)
                {
                    sum += L(i,j) * U(j,k);
                }
                U(i,k) = (*this)(i,k) - sum;
            }
        

            //Lower Triangular - L column i
            for(int k = i; k < n; k++)
            {
                if( i == k){
                    L(i,i) = T(1);
                }
                else{
                    T sum = T(0);
                    for(int j = 0; j < i; j++)
                    {
                        sum += L(k,j) * U(j,i);
                    }
                    if(U(i,i) == T(0)) return false; //cannot decompose
                    L(k,i) = ((*this)(k,i) - sum) / U(i,i);
                }
            }
        }
        return true;
    }

    template <typename T>
    Vector<T> Matrix<T> :: solve(const Vector<T>& b) const{
        if(rows_ != cols_){
            throw std :: invalid_argument("Solve requires square matrix");
        }

        if(b.size() != rows_)
        {
            throw std::invalid_argument("Vector size doesn't match matrix");
        }

        int n = rows_;
        Matrix<T> L(n,n), U(n,n);
        if(!lu_decompose(L,U))
        {
            throw std::runtime_error("LU decomposition failed");
        }

        /*  Forward substitution — L is lower triangular, so L * y = b can be solved top to bottom. The first equation has only y(0), the second has y(0) and y(1) (and we already know y(0)), and so on.
            Back substitution — U is upper triangular, so U * x = y is solved bottom to top. The last equation has only x(n-1), then we work backwards.
        */

        //Forward Subsitution: Ly = b
        Vector<T> y(n);
        for(int i = 0; i< n;i++)
        {
            T sum = T(0);
            for(int j = 0; j < i;j++)
            {
                sum += L(i,j) * y(j);
            }

            y(i) = (b(i) - sum) / L(i,i);
        }

        //Back subsitution: Ux = y
        Vector<T> x(n);
        for(int i = n-1; i>=0;i--)
        {
            T sum = T(0);
            for(int j = i + 1; j < n;j++)
            {
                sum += U(i,j) * x(j);
            }
            x(i) = (y(i) - sum) /U(i,i);
        }
        return x;
    }
    template <typename T>
    Matrix<T> Matrix<T>::operator*(const T& s) const {
        Matrix<T> result(rows_, cols_);     // same shape
        // single loop over all rows_*cols_ elements:
        for(int i = 0; i < (rows_ * cols_); i++)
        {
            result.data_[i] = data_[i] * s;
        }
        //   result.data_[idx] = data_[idx] * s;
        return result;
    }
    template <typename T>
    Vector<T> Matrix<T>::operator*(const Vector<T>& v) const {
        // dimension check: cols_ must equal v's length — throw if not (your method #1 habit)
        Vector<T> result(rows_);              // length = number of rows
        // double loop: for each row i, sum over columns j of A(i,j) * v(j)
        for (int i = 0; i < rows_; ++i) {
            T sum = T();                          // zero-init (works for double AND complex)
            for (int j = 0; j < cols_; ++j) {
                sum += data_[i * cols_ + j] * v(j);
            }
            result(i) = sum;
        }
        return result;
    }
    template <typename T>
    Vector<T> Matrix<T>::solve_with_LU(const Matrix<T>& L, const Matrix<T>& U,
                                    const Vector<T>& b) {
        int n = L.rows_;

        // Forward substitution: L y = b
        Vector<T> y(n);
        for (int i = 0; i < n; i++) {
            T sum = T(0);
            for (int j = 0; j < i; j++) {
                sum += L(i, j) * y(j);
            }
            y(i) = (b(i) - sum) / L(i, i);
        }

        // Back substitution: U x = y
        Vector<T> x(n);
        for (int i = n - 1; i >= 0; i--) {
            T sum = T(0);
            for (int j = i + 1; j < n; j++) {
                sum += U(i, j) * x(j);
            }
            x(i) = (y(i) - sum) / U(i, i);
        }
        return x;
    }
}

#endif