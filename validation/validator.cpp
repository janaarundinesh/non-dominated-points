#include "validator.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

ValidationDataset LoadDataset(const std::string& filename)
{
    ValidationDataset dataset;

    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error opening file: "
                  << filename << std::endl;
        return dataset;
    }

    std::string line;

    // ---------- Dimension ----------
    while (std::getline(file, line))
    {
        if (!line.empty() && line[0] != '#')
        {
            dataset.dimension = std::stoul(line);
            break;
        }
    }

    // ---------- Number of Input Points ----------
    size_t inputCount = 0;

    while (std::getline(file, line))
    {
        if (!line.empty() && line[0] != '#')
        {
            inputCount = std::stoul(line);
            break;
        }
    }

    // ---------- Input Points ----------
    for (size_t i = 0; i < inputCount;)
    {
        std::getline(file, line);

        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);

        Item item;

        int value;

        while (ss >> value)
        {
            item.coords.push_back(value);
        }

        dataset.inputPoints.push_back(item);

        i++;
    }

    // ---------- Number of Expected Maxima ----------
    size_t expectedCount = 0;

    while (std::getline(file, line))
    {
        if (!line.empty() && line[0] != '#')
        {
            expectedCount = std::stoul(line);
            break;
        }
    }

    // ---------- Expected Maxima ----------
    for (size_t i = 0; i < expectedCount;)
    {
        std::getline(file, line);

        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);

        Item item;

        int value;

        while (ss >> value)
        {
            item.coords.push_back(value);
        }

        dataset.expectedMaxima.push_back(item);

        i++;
    }

    return dataset;
}