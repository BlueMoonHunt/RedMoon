#include "activations.h"
#include <cmath>

namespace rm {
    double sigmoid(double x) {
        return 1.0 / (1.0 + std::exp(-x));
    }

    double relu(double x) {
        return std::max(0.0, x);
    }

    float sigmoidf(float x) {
        return 1.0f / (1.0f + std::exp(-x));
    }

    float reluf(float x) {
        return std::max(0.0f, x);
    }
} // namespace rm