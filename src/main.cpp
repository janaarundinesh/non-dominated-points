#include <iostream>
#include "item.hpp"
#include "generate_data.hpp"
#include "print.hpp"
#include "sorting.hpp"
#include "partition.hpp"
#include "threshold.hpp"
#include "split_u.hpp"
#include "intersection.hpp"
#include "filter.hpp"

using namespace std;

int main()
{
    size_t Dim = 6, NPoints = 10;
    vector<Item> items = generateData(NPoints, Dim);

    /* Prints the Data Generated*/
    printItems(items);

    // /* Sorts the Last Co-ordinates in Asc Order*/
    // SortByLastCoordinate(items);

    auto [V1, V2] = PartitionV(items);

    cout << "\nV1:\n";
    printItems(V1);

    cout << "\nV2:\n";
    printItems(V2);

    int threshold =
    ThresholdFromV1(V1, Dim);

    cout << "\nThreshold = " << threshold << endl;

    auto [U1,U2] = SplitU(items, threshold, Dim);

    cout << "\nU1:\n";
    printItems(U1);

    cout << "\nU2:\n";
    printItems(U2);

    auto R = FILTER(U,V,3);

    cout << "\nFILTER:\n";
    printItems(R);

    return 0; 
};