#include "perlin_noise_2D.hpp"
#include <cmath>

PerlinNoise2D::PerlinNoise2D() : Random2DCoordinateGenerator() {}

float PerlinNoise2D::operator()(const float& x, const float& y) const
{
    return sample(x, y);
}

float PerlinNoise2D::sample(const float& x, const float& y) const
{
    long long i0 = static_cast<long long>(std::floor(x));
    long long j0 = static_cast<long long>(std::floor(y));
    long long i1 = i0 + 1;
    long long j1 = j0 + 1;

    float u = x - i0;
    float v = y - j0;

    long long hash00 = Random2DCoordinateGenerator::operator()(i0, j0);
    long long hash10 = Random2DCoordinateGenerator::operator()(i1, j0);
    long long hash01 = Random2DCoordinateGenerator::operator()(i0, j1);
    long long hash11 = Random2DCoordinateGenerator::operator()(i1, j1);

    float dot00 = grad(hash00, u, v);
    float dot10 = grad(hash10, u-1, v);
    float dot01 = grad(hash01, u, v-1);
    float dot11 = grad(hash11, u-1, v-1);

    float fu = fade(u);
    float fv = fade(v);

    float lerpX0 = lerp(dot00, dot10, fu);
    float lerpX1 = lerp(dot01, dot11, fu);

    float value = lerp(lerpX0, lerpX1, fv);

    return value;
}

float PerlinNoise2D::fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise2D::lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

float PerlinNoise2D::grad(long long hash, float dx, float dy)
{
    switch (hash & 7)
    {
        case 0: return  dx + dy;
        case 1: return -dx + dy;
        case 2: return  dx - dy;
        case 3: return -dx - dy;
        case 4: return  dx;
        case 5: return -dx;
        case 6: return  dy;
        case 7: return -dy;
    }
    return 0.0f;
}