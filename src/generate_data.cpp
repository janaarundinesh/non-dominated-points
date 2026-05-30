#include "generate_data.hpp"
#include <random>

using namespace std;

vector<Item> generateData()
{
    vector<Item> items;

    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> weight_dist(1,20);
    uniform_int_distribution<> profit_dist(1, 30);

    for (int i = 0 ; i < 10 ; i++){
        Item item;

        item.weight = weight_dist(gen);
        item.profit = profit_dist(gen);

        items.push_back(item);
    }
    return items;
}