#pragma once
#include <cmath>

template <typename T>
class IVector2
{
public:
    T x;
    T y;

    IVector2 operator+(const IVector2<T>& other) const;
    IVector2 operator-(const IVector2<T>& other) const;
    IVector2 operator*(const IVector2<T>& other) const;
    IVector2 operator/(const IVector2<T>& other) const;
    bool operator==(const IVector2<T>& other) const;
    bool operator!=(const IVector2<T>& other) const;

    float length() const;
    IVector2<float> normalize() const;
    float dot(const IVector2<T>& other) const;
    IVector2<T> cross() const;
};

#include "ivector2.tpp"