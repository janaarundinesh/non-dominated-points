#include "validator.hpp"

bool ValidateMaxima(const std::vector<Item>& originalItems, const std::vector<Item>& maxima, size_t dimension) {
    // Check if each point in maxima is non-dominated by any other point in originalItems
    for (const auto& maxPoint : maxima) {
        for (const auto& origPoint : originalItems) {
            if (origPoint != maxPoint && dominates(origPoint, maxPoint, dimension)) {
                return false; // Found a point that dominates a maxima point
            }
        }
    }
    return true; // All maxima points are valid
}