// Dumps the worst-case input as a dataset file, so the MPI programs can be
// measured on exactly the antichain worst_case_benchmark uses: the same
// generator, the same seed.
//
// Usage: antichain_dataset <output.txt> <dimensions> <num_points>

#include <fstream>
#include <iostream>
#include <string>

#include "antichain.hpp"
#include "point_set.hpp"

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <output.txt> <dimensions> <num_points>\n";

        return 1;
    }

    const size_t dimensions = std::stoul(argv[2]);
    const size_t num_points = std::stoul(argv[3]);

    if (dimensions < 2 || num_points < 1)
    {
        std::cerr << "Need at least two dimensions and one point.\n";

        return 1;
    }

    const PointSet set = BuildAllMaximaSet(dimensions, num_points, 20260124u);

    if (!IsValid(set))
    {
        std::cerr << "Generated point set is invalid.\n";

        return 1;
    }

    std::ofstream output(argv[1]);

    if (!output)
    {
        std::cerr << "Could not open output file: " << argv[1] << '\n';

        return 1;
    }

    // The dataset format the loaders read: a "d N" header, then one point per
    // line. Coordinates are whole numbers, so they are printed without a
    // fractional part and stay exact.
    output << dimensions << ' ' << num_points << '\n';

    for (const auto& point : set.points)
    {
        for (size_t d = 0; d < point.coords.size(); ++d)
        {
            if (d != 0)
            {
                output << ' ';
            }

            output << static_cast<long long>(point.coords[d]);
        }

        output << '\n';
    }

    std::cout << "wrote " << num_points << " points in " << dimensions
              << "D to " << argv[1] << '\n';

    return 0;
}
