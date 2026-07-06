#include <iostream>
#include <vector>
#include "item.hpp"
#include "generate_data.hpp"
#include "print.hpp"
#include "maxima2.hpp"
#include <chrono>
#include <fstream>

using namespace std;

int main()
{
    //size_t Dim = 10, NPoints = 1000;

    const vector<size_t> dims = {2, 3, 4, 5, 6, 7, 8, 9, 10};

    const vector<size_t> npoints = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};

    ofstream csv_file("../results/benchmark_results.csv");
    csv_file << "Dimension,NPoints,ExecutionTime_us\n";

    for (size_t Dim : dims)
    {
        for (size_t NPoints : npoints)
        {
            //repeat 10 times and take average time
            vector<double> execution_times;
            const int num_iterations = 10;

            vector<Item> Originalitems = generateData(NPoints, Dim);

            for (int i = 0; i < num_iterations; ++i)
            {
                vector<Item> items = Originalitems; // Copy the original items for each iteration

                auto start = chrono::high_resolution_clock::now();

                //cout << "Generated " << NPoints << " points in " << Dim << "D:\n";
                //printItems(items);

                auto maxima = MAXIMA2(items, Dim);

                //cout << "\nNon-dominated points:\n";
                //printItems(maxima);

                auto stop = chrono::high_resolution_clock::now();

                double elapsed = chrono::duration<double, micro>(stop - start).count();
                //cout << "Time taken: " << elapsed << " microseconds\n";
                execution_times.push_back(elapsed);
            }
            // Calculate average time
            double average_time = 0.0;
            for (double t : execution_times)
            {
                average_time += t;
            }
            average_time /= execution_times.size();
            csv_file << Dim << "," << NPoints << "," << average_time << "\n";
        }
    }
    csv_file.close();
    cout << "Benchmark results saved to ../results/benchmark_results.csv\n";
    cout << "\nGenerating Plots...\n";

    int status = system("python3 ../plots/plot_results.py");

    if (status == 0)
    {
        cout << "Plots generated successfully.\n";
    }
    else
    {
        cout << "Error generating plots.\n";
    }
    return 0;
};