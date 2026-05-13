#pragma once
#include "random_2D_coordinate_generator.hpp"

class PerlinNoise2D : Random2DCoordinateGenerator
{
public:
    PerlinNoise2D();

    float operator()(const float& x, const float& y) const;

    float sample(const float& x, const float& y) const;

private:
    static float fade(float t);
    static float lerp(float a, float b, float t);
    static float grad(long long hash, float dx, float dy);
};