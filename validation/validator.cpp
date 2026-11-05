#include "validator.hpp"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace {

// Dataset files separate sections with blank lines and '#' comments. Returns
// the next line that carries data.
bool NextDataLine(std::istream& file, std::string& line)
{
    while (std::getline(file, line))
    {
        if (line.find_first_not_of(" \t\r") == std::string::npos)
        {
            continue;
        }

        if (line[0] == '#')
        {
            continue;
        }

        return true;
    }

    return false;
}

std::optional<size_t> ReadCount(std::istream& file)
{
    std::string line;

    if (!NextDataLine(file, line))
    {
        return std::nullopt;
    }

    std::istringstream fields(line);
    size_t value = 0;

    if (!(fields >> value))
    {
        return std::nullopt;
    }

    fields >> std::ws;

    if (!fields.eof())
    {
        return std::nullopt;
    }

    return value;
}

// Reads `count` point lines, each carrying exactly `dimension` coordinates.
bool ReadPoints(
    std::istream& file,
    size_t dimension,
    size_t count,
    std::vector<Item>& points
)
{
    for (size_t i = 0; i < count; ++i)
    {
        std::string line;

        if (!NextDataLine(file, line))
        {
            return false;
        }

        std::istringstream fields(line);

        Item item;
        double coordinate = 0.0;

        while (fields >> coordinate)
        {
            item.coords.push_back(coordinate);
        }

        if (!fields.eof() || item.coords.size() != dimension)
        {
            return false;
        }

        points.push_back(item);
    }

    return true;
}

} // namespace

std::optional<ValidationDataset> LoadDataset(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        std::cerr << "Could not open dataset: " << filename << '\n';

        return std::nullopt;
    }

    ValidationDataset dataset;

    const std::optional<size_t> dimension = ReadCount(file);

    if (!dimension || *dimension == 0)
    {
        std::cerr << "Missing or invalid dimension: " << filename << '\n';

        return std::nullopt;
    }

    dataset.dimension = *dimension;

    const std::optional<size_t> input_count = ReadCount(file);

    if (!input_count)
    {
        std::cerr << "Missing or invalid input point count: " << filename << '\n';

        return std::nullopt;
    }

    if (!ReadPoints(file, dataset.dimension, *input_count, dataset.inputPoints))
    {
        std::cerr << "Malformed input points: " << filename << '\n';

        return std::nullopt;
    }

    const std::optional<size_t> expected_count = ReadCount(file);

    if (!expected_count)
    {
        std::cerr << "Missing or invalid expected maxima count: " << filename << '\n';

        return std::nullopt;
    }

    if (!ReadPoints(file, dataset.dimension, *expected_count, dataset.expectedMaxima))
    {
        std::cerr << "Malformed expected maxima: " << filename << '\n';

        return std::nullopt;
    }

    return dataset;
}
