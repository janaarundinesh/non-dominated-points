#include <iostream>

#include "validator.hpp"
#include "maxima2.hpp"
#include "print.hpp"

using namespace std;

bool CompareMaxima(const std::vector<Item>& computed, const std::vector<Item>& expected)
{
    if (computed.size() != expected.size())
    {
        return false;
    }

    for (size_t i = 0; i < computed.size(); i++)
    {
        if (computed[i].coords != expected[i].coords)
            return false;
    }

    return true;
}

int main()
{
    int passed = 0;
    for (int i = 1; i <= 13; i++)
    {
        ValidationDataset dataset = LoadDataset("../validation/test_datasets/Test" + std::to_string(i) + ".txt");

        auto maxima = MAXIMA2(dataset.inputPoints, dataset.dimension);

        if (CompareMaxima(maxima, dataset.expectedMaxima))
        {
            cout << "Test Case " << i << " : PASSED\n\n";
            passed++;
        }
        else
        {
            cout << "Test Case " << i << " : FAILED\n";

            cout << "\nExpected Maxima\n";
            printItems(dataset.expectedMaxima);

            cout << "\nComputed Maxima\n";
            printItems(maxima);

            cout << "\n";
        }
    }
    cout << "\n=====================================\n";
    cout << "Validation Summary\n";
    cout << "=====================================\n";
    cout << "Passed : " << passed << " / 13\n";


    return 0;
}