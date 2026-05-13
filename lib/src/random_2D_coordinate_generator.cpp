#include "random_2D_coordinate_generator.hpp"
#include <random>
#include <functional>

Random2DCoordinateGenerator::Random2DCoordinateGenerator()
{
    std::random_device rd;
    _seed = rd();
}

long long Random2DCoordinateGenerator::operator()(const long long& x, const long long& y) const
{
    std::seed_seq seq{static_cast<unsigned long>(_seed), static_cast<unsigned long>(x), static_cast<unsigned long>(y)};
    
    std::mt19937_64 prng_local(seq);
    return prng_local();
}

long long Random2DCoordinateGenerator::seed() const
{
    return _seed;
}