#include <iostream>
#include <vector>
#include "item.hpp"
#include "print.hpp"
#include "maxima2.hpp"
#include <chrono>
#include <fstream>
#include <filesystem>

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <dataset_file>\n";
        return 1;
    }

    string filename = argv[1];

    ifstream file(filename);

    if (!file)
    {
        cerr << "Could not open dataset: " << filename << '\n';
        return 1;
    }

    size_t Dim, NPoints;

    file >> Dim >> NPoints;

    vector<Item> items(NPoints);

    for (size_t i = 0; i < NPoints; ++i)
    {
        items[i].coords.resize(Dim);

        for (size_t d = 0; d < Dim; ++d)
        {
            file >> items[i].coords[d];
        }
    }

    file.close();

    cout << "Dataset: " << filename << '\n';
    cout << "Dimension: " << Dim << '\n';
    cout << "Number of points: " << NPoints << '\n';

    auto start = chrono::high_resolution_clock::now();

    auto maxima = MAXIMA2(items, Dim);

    auto stop = chrono::high_resolution_clock::now();

    double elapsed = chrono::duration<double, micro>(stop - start).count();

    cout << "Non-dominated points: " << maxima.size() << '\n';
    cout << "Execution time: " << elapsed << " us\n";

    return 0;
}