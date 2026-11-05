#include "dataset.hpp"

#include <fstream>
#include <iostream>

std::optional<PointSet> LoadPointSet(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        std::cerr << "Could not open dataset: " << filename << '\n';

        return std::nullopt;
    }

    PointSet set;

    size_t num_points = 0;

    if (!(file >> set.dimensions >> num_points))
    {
        std::cerr << "Could not read dataset header: " << filename << '\n';

        return std::nullopt;
    }

    if (set.dimensions == 0)
    {
        std::cerr << "Dataset has no dimensions: " << filename << '\n';

        return std::nullopt;
    }

    set.points.resize(num_points);

    for (auto& point : set.points)
    {
        point.coords.resize(set.dimensions);

        for (auto& coordinate : point.coords)
        {
            if (!(file >> coordinate))
            {
                std::cerr << "Dataset ended early: " << filename << '\n';

                return std::nullopt;
            }
        }
    }

    if (!IsValid(set))
    {
        std::cerr << "Dataset is inconsistent: " << filename << '\n';

        return std::nullopt;
    }

    return set;
}
