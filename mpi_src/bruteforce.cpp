#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstdint>

//dominance function

int Dominates(const double* point_a, const double* point_b, uint64_t dimensions)
{
    bool a_strictly_better = false;
    bool b_strictly_better = false;

    for (uint64_t d = 0; d < dimensions; ++d)
    {
        if (point_a[d] > point_b[d])
        {
            a_strictly_better = true;
        }
        else if (point_a[d] < point_b[d])
        {
            b_strictly_better = true;
        }
    }

    if (a_strictly_better && !b_strictly_better)
        return 1;

    if (b_strictly_better && !a_strictly_better)
        return -1;

    return 0;
}

// Function to compute the non-dominated points using brute-force method

std::vector<double> BruteForceMaxima(
    const std::vector<double>& data,
    uint64_t num_points,
    uint64_t dimensions)
{
    std::vector<double> result;

    for (uint64_t i = 0; i < num_points; ++i)
    {
        bool dominated = false;

        const double* point_i = &data[i * dimensions];

        for (uint64_t j = 0; j < num_points; ++j)
        {
            if (i == j)
                continue;

            const double* point_j = &data[j * dimensions];

            int result_dominance =
                Dominates(point_j, point_i, dimensions);

            if (result_dominance == 1)
            {
                dominated = true;
                break;
            }
        }

        if (!dominated)
        {
            result.insert(result.end(),point_i,point_i + dimensions);
        }
    }

    return result;
}

int main()
{
    uint64_t dimensions = 0;
    uint64_t num_points = 0;

    std::string filename = "../DataSets/10D_Data/Test6.txt"; // Specify the dataset file path

    std::ifstream file(filename);

    if (!file)
    {
        std::cerr << "Could not open dataset: "
                  << filename << std::endl;

        return 1;
    }

    file >> dimensions >> num_points; // Read dimensions and number of points

    std::vector<double> all_data(num_points * dimensions);

    // Read the data points from the file

    for (uint64_t i = 0; i < num_points; ++i)
    {
        for (uint64_t j = 0; j < dimensions; ++j)
        {
            file >> all_data[i * dimensions + j];
        }
    }

    file.close();

    // calculate the time taken for the non-dominated points calculation
    auto start_time = std::chrono::steady_clock::now();

    std::vector<double> maxima = BruteForceMaxima(all_data,num_points,dimensions);

    auto end_time = std::chrono::steady_clock::now();

    double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();

    uint64_t total_non_dominated = maxima.size() / dimensions;

    std::cout << "Elapsed time : " << elapsed_time << " seconds" << std::endl;
    std::cout << "Dimensions: " << dimensions << std::endl;
    std::cout << "Number of points: " << num_points << std::endl;
    std::cout << "Number of non-dominated points: " << total_non_dominated << std::endl;

    return 0;
}