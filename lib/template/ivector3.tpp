template <typename T>
IVector3<T> IVector3<T>::operator+(const IVector3<T>& other) const
{
    return IVector3<T>{ x + other.x, y + other.y, z + other.z };
}

template <typename T>
IVector3<T> IVector3<T>::operator-(const IVector3<T>& other) const
{
    return IVector3<T>{ x - other.x, y - other.y, z - other.z };
}

template <typename T>
IVector3<T> IVector3<T>::operator*(const IVector3<T>& other) const
{
    return IVector3<T>{ x * other.x, y * other.y, z * other.z };
}

template <typename T>
IVector3<T> IVector3<T>::operator/(const IVector3<T>& other) const
{
    return IVector3<T>{ x / other.x, y / other.y, z / other.z };
}

template <typename T>
bool IVector3<T>::operator==(const IVector3<T>& other) const
{
    return x == other.x && y == other.y && z == other.z;
}

template <typename T>
bool IVector3<T>::operator!=(const IVector3<T>& other) const
{
    return x != other.x || y != other.y || z != other.z;
}

template <typename T>
float IVector3<T>::length() const
{
    return std::sqrt(x * x + y * y + z * z);
}

template <typename T>
IVector3<float> IVector3<T>::normalize() const
{
    float len = length();
    if (len == 0.0f) return IVector3<float>{0.0f, 0.0f, 0.0f};
    return IVector3<float>{ static_cast<float>(x) / len, static_cast<float>(y) / len, static_cast<float>(z) / len };
}

template <typename T>
float IVector3<T>::dot(const IVector3<T>& other) const
{
    return static_cast<float>(x * other.x + y * other.y + z * other.z);
}

template <typename T>
IVector3<T> IVector3<T>::cross(const IVector3<T>& other) const
{
    return IVector3<T>{ y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
}