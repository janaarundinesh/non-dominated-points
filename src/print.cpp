#include <iostream>
#include "print.hpp"

void printItems(const std::vector<Item>& items)
{
    for (const auto& item : items)
    {
        std::cout << "(";

        for (size_t i = 0; i < item.coords.size(); i++)
        {
            std::cout << item.coords[i];

            if (i < item.coords.size() - 1)
                std::cout << ", ";
        }

        std::cout << ")\n";
    }
    std::cout << items.size() << " points total.\n";
}