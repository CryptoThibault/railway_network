#include "train_parameters.hpp"
#include <cmath>
#include <initializer_list>
#include <stdexcept>

void TrainParameters::validate() const
{
    for (double value : {mass, maxSpeed, engineForce, brakeForce, length})
        if (!std::isfinite(value) || value <= 0.0)
            throw std::invalid_argument("Train physical parameters must be positive and finite");
    if (!std::isfinite(friction) || friction < 0.0)
        throw std::invalid_argument("Train friction must be nonnegative and finite");
    if (!std::isfinite(engineForce / mass) || engineForce / mass <= 0.0
        || !std::isfinite(brakeForce / mass) || brakeForce / mass <= 0.0)
        throw std::invalid_argument("Invalid train acceleration or braking capacity");
}
