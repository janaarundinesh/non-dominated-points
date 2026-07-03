#include <iostream>
#include "print.hpp"

using namespace std;

void printItems(const vector<Item>& items)
{
    for (const auto& item : items)
    {
        cout << "(";

        for (size_t i = 0; i < item.coords.size(); i++)
        {
            cout << item.coords[i];

            if (i < item.coords.size() - 1)
                cout << ", ";
        }

        cout << ")\n";
    }
    cout << items.size() << " points total.\n";
}