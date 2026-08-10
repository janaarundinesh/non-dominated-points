#include "generate_data.hpp"
#include <iostream>
#include <random>
#include <fstream>

using namespace std;

vector<Item> generateData(size_t n ,size_t D)
{
    vector<Item> items;

    random_device rd;
    mt19937 gen(rd());

    uniform_real_distribution<double> range(0.0,1.0);

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
}

void saveData(const vector<Item>& items, const string& filename)
{
    ofstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    file << items.size() << endl;

    for (const auto& item : items)
    {
        for (size_t i = 0; i < item.coords.size(); i++)
        {
            file << item.coords[i];
            if (i < item.coords.size() - 1)
                file << " ";
        }
        file << endl;
    }

    file.close();
};