#include "generate_data.hpp"
#include <random>

using namespace std;

vector<Item> generateData(size_t n ,size_t D)
{
    vector<Item> items;

    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> range(1,100);

    for (size_t i = 0; i < n; i++)
    {
        Item item;

        for (size_t j = 0; j < D; j++)
        {
            item.coords.push_back(range(gen));
        }

        items.push_back(item);

    }

    return items;
};