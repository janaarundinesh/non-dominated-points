#include <iostream>

#include "validator.hpp"
#include "maxima2.hpp"
#include "print.hpp"

using namespace std;

int main()
{
    for (int i = 1; i <= 10; i++)
    {
        ValidationDataset dataset =LoadDataset("../validation/test_datasets/test" + std::to_string(i) + ".txt");

        cout << "==================== Test Dataset " << i << " ====================\n";

        cout << "Dimension : " << dataset.dimension << "\n\n";

        cout << "Input Points\n";
        printItems(dataset.inputPoints);

        auto maxima = MAXIMA2(dataset.inputPoints, dataset.dimension);

        cout << "\nComputed Maxima\n";
        printItems(maxima);

        cout << "\nExpected Maxima\n";
        printItems(dataset.expectedMaxima);
    }


    return 0;
}