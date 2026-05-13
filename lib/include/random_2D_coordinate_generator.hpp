#pragma once

class Random2DCoordinateGenerator
{
public:
    Random2DCoordinateGenerator();

    long long operator()(const long long& x, const long long& y) const;
    
    long long seed() const;

private:
    long long _seed;
};