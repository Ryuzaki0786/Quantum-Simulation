#ifndef VECTOR_H
#define VECTOR_H

#include <vector>
#include <iostream>
#include <stdexcept>
#include <complex>
#include <cmath>

namespace quantum
{
    template<typename T>
    class Vector
    {
        private:
            int size_;
            std::vector<T> data_;
        public:
            Vector(int size);
            Vector(int size,T initial_value);
        
            T& operator()(int i);
            const T& operator()(int i) const;

            int size() const {return size_;}

            Vector<T> operator+(const Vector<T>& other) const;
            Vector<T> operator-(const Vector<T>& other) const;
            Vector<T> operator*(T scalar) const;

            T dot(const Vector<T>& other) const;
            double norm() const;
            Vector<T> normalize() const;

            void print() const;

    };
    template<typename T>
    Vector<T> :: Vector(int size):
    size_(size),data_(size,T(0)) {}

    template<typename T>
    Vector<T> :: Vector(int size, T initial_value):
    size_(size),data_(size,initial_value){}

    template<typename T>
    T& Vector<T> :: operator()(int i)
    {
        return data_[i];
    }
    template<typename T>
    const T& Vector<T> :: operator()(int i) const {
        return data_[i];
    }

    template<typename T>
    Vector<T> Vector<T> :: operator+(const Vector<T>& other) const{
        Vector<T> result(size_);
        if(size_ != other.size_)
        {
            throw std::invalid_argument("Vector dimensions do not match for addition");
        }
        for(int i = 0; i < size_; i++)
        {
            result.data_[i] = data_[i] + other.data_[i];
        }
        return result;
    }
    template<typename T>
    Vector<T> Vector<T> :: operator-(const Vector<T>& other) const{
        Vector<T> result(size_);
        if(size_ != other.size_)
        {
            throw std::invalid_argument("Vector dimensions do not match for subtraction");
        }
        for(int i = 0; i < size_; i++)
        {
            result.data_[i] = data_[i] - other.data_[i];
        }
        return result;
    }
    template<typename T>
    Vector<T> Vector<T> :: operator*(T scalar) const{
        Vector<T> result(size_);
        for(int i = 0; i < size_; i++)
        {
            result.data_[i] = data_[i] * scalar;
        }
        return result;
    }

    template<typename T>
    T Vector<T> :: dot(const Vector<T>& other) const
    {
        if (size_ != other.size_) {
            throw std::invalid_argument("Vector dimensions don't match for dot product");
        }
        T result = T(0);
        for(int i = 0;i < size_;i++)
        {
            result += (data_[i] * other.data_[i]);
        }

        return result;
    }

    template<typename T>
    double Vector<T>::norm() const {
        double sum = 0.0;
        #pragma omp parallel for reduction(+:sum) if(size_ > 100000)
        for (int i = 0; i < size_; i++) {
            sum += std::abs(data_[i]) * std::abs(data_[i]);
        }
        return std::sqrt(sum);
    }

    template<typename T>
    Vector<T> Vector<T>::normalize() const {
        double n = norm();
        if (n == 0) {
            throw std::runtime_error("Cannot normalize zero vector");
        }
        Vector<T> result(size_);
        #pragma omp parallel for
        for (int i = 0; i < size_; i++) {
            result.data_[i] = data_[i] / n;
        }
        return result;
    }

    template <typename T>
    void Vector<T>::print() const{
        for(int i = 0; i < size_;i++)
        {
            std::cout <<data_[i]<< " ";
        }
        std::cout << std :: endl;
    }

}

#endif