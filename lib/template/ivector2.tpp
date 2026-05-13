template <typename T>
IVector2<T> IVector2<T>::operator+(const IVector2<T>& other) const
{
    return IVector2<T>{ x + other.x, y + other.y };
}

template <typename T>
IVector2<T> IVector2<T>::operator-(const IVector2<T>& other) const
{
    return IVector2<T>{ x - other.x, y - other.y };
}

template <typename T>
IVector2<T> IVector2<T>::operator*(const IVector2<T>& other) const
{
    return IVector2<T>{ x * other.x, y * other.y };
}

template <typename T>
IVector2<T> IVector2<T>::operator/(const IVector2<T>& other) const
{
    return IVector2<T>{ x / other.x, y / other.y };
}

template <typename T>
bool IVector2<T>::operator==(const IVector2<T>& other) const
{
    return x == other.x && y == other.y;
}

template <typename T>
bool IVector2<T>::operator!=(const IVector2<T>& other) const
{
    return x != other.x || y != other.y;
}

template <typename T>
float IVector2<T>::length() const
{
    return std::sqrt(x * x + y * y);
}

template <typename T>
IVector2<float> IVector2<T>::normalize() const
{
    float len = length();
    if (len == 0.0f) return IVector2<float>{0.0f, 0.0f};
    return IVector2<float>{ static_cast<float>(x) / len, static_cast<float>(y) / len };
}

template <typename T>
float IVector2<T>::dot(const IVector2<T>& other) const
{
    return static_cast<float>(x * other.x + y * other.y);
}

template <typename T>
IVector2<T> IVector2<T>::cross() const
{
    return IVector2<T>{ -y, x };
}