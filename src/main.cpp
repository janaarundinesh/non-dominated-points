#include <iostream>
#include "item.hpp"
#include "generate_data.hpp"
#include "print.hpp"
#include "sorting.hpp"

using namespace std;

int main()
{
    vector<Item> items = generateData(6, 10);

    /* Prints the Data Generated*/
    printItems(items);

    /* Sorts the Last Co-ordinates in Asc Order*/
    SortByLastCoordinate(items);

    printItems(items);

    return 0;
};