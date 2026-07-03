#include <iostream>
#include "item.hpp"
#include "generate_data.hpp"
#include "print.hpp"
#include "maxima2.hpp"

using namespace std;

int main()
{
    size_t Dim = 3, NPoints = 12;

    vector<Item> items = generateData(NPoints, Dim);

    cout << "Generated " << NPoints << " points in " << Dim << "D:\n";
    printItems(items);

    auto maxima = MAXIMA2(items, Dim);

    cout << "\nNon-dominated points:\n";
    printItems(maxima);

    return 0;
}
