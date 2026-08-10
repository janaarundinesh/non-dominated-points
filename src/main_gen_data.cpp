#include "generate_data.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    const std::vector<size_t> dimensions = {5, 6, 7, 8, 9, 10};
    const std::vector<size_t> numItems = {50, 100, 500, 1000, 10000};

    const std::string outputDir = "../DataSets";

    for (const auto& D : dimensions)
    {
        std::string dimensionDirectory = outputDir + "/" + std::to_string(D) + "D_Data";

        std::filesystem::create_directories(dimensionDirectory);

        for (size_t test = 0; test < numItems.size(); ++test)
        {
            size_t numberOfPoints = numItems[test];

            std::string filename = dimensionDirectory + "/Test" + std::to_string(test + 1) + ".txt";

            std::vector<Item> items = generateData(numberOfPoints, D);

            saveData(items, filename);

            std::cout << "Generated and saved data for " << numberOfPoints << " items in " << D << " dimensions to " << filename << std::endl;

        }

    }
    std::cout << "All Datasets are generated and saved " << std::endl;

    return 0;
}