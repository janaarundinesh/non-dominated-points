#pragma once

#include <cstddef>
#include <vector>

#include "item.hpp"

// MPI moves flat arrays of doubles, while the library works with Item objects.
// These conversions live in one header so the MPI programs do not each carry
// their own copy of them.

inline std::vector<double> FlattenItems(
    const std::vector<Item>& items,
    size_t dimensions)
{
    std::vector<double> data(items.size() * dimensions);

    for (size_t i = 0; i < items.size(); ++i)
    {
        for (size_t j = 0; j < dimensions; ++j)
        {
            data[i * dimensions + j] = items[i].coords[j];
        }
    }

    return data;
}

inline std::vector<Item> UnflattenItems(
    const std::vector<double>& data,
    size_t number_of_items,
    size_t dimensions)
{
    std::vector<Item> items(number_of_items);

    for (size_t i = 0; i < number_of_items; ++i)
    {
        items[i].coords.resize(dimensions);

        for (size_t j = 0; j < dimensions; ++j)
        {
            items[i].coords[j] = data[i * dimensions + j];
        }
    }

    return items;
}
