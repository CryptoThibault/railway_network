#pragma once
#include <cmath>

template <typename T>
class IVector3
{
public:
    T x;
    T y;
    T z;

    IVector3 operator+(const IVector3<T>& other) const;
    IVector3 operator-(const IVector3<T>& other) const;
    IVector3 operator*(const IVector3<T>& other) const;
    IVector3 operator/(const IVector3<T>& other) const;
    bool operator==(const IVector3<T>& other) const;
    bool operator!=(const IVector3<T>& other) const;

    float length() const;
    IVector3<float> normalize() const;
    float dot(const IVector3<T>& other) const;
    IVector3<T> cross(const IVector3<T>& other) const;
};

#include "ivector3.tpp"