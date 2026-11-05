#pragma once

#include <optional>
#include <string>

#include "point_set.hpp"

// Reads the dataset format used by DataSets/: a header line "d N" followed by N
// lines of d coordinates.
//
// Returns std::nullopt and explains the reason on stderr when the file cannot
// be opened, ends early, or describes an inconsistent point set.
std::optional<PointSet> LoadPointSet(const std::string& filename);
