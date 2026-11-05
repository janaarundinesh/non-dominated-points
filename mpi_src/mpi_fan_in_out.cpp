// Fan-out / fan-in method for the maxima problem.
//
// Rank 0 scatters the data, every process computes the maxima of its share, and
// the partial results are combined up a reduction tree.
//
// Usage: MPI_FanInOut <dataset_file>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <mpi.h>

#include "brute_force_maxima.hpp"
#include "dominance.hpp"
#include "mpi_items.hpp"

namespace {

// Both inputs are individually non-dominated, so only the cross pairs matter.
std::vector<Item> MergeMaxima(
    const std::vector<Item>& existing,
    const std::vector<Item>& incoming,
    size_t d)
{
    std::vector<Item> result;
    result.reserve(existing.size() + incoming.size());

    // Keep existing points that are not dominated by any incoming point
    for (const auto& e : existing)
    {
        bool dominated = false;

        for (const auto& n : incoming)
        {
            if (Dominates(n, e, d))
            {
                dominated = true;
                break;
            }
        }

        if (!dominated)
        {
            result.push_back(e);
        }
    }

    // Keep incoming points that are not dominated by any existing point
    for (const auto& n : incoming)
    {
        bool dominated = false;

        for (const auto& e : existing)
        {
            if (Dominates(e, n, d))
            {
                dominated = true;
                break;
            }
        }

        if (!dominated)
        {
            result.push_back(n);
        }
    }

    return result;
}

} // namespace

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank = 0;
    int world = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    const size_t world_size = static_cast<size_t>(world);

    // ========================================================
    // 1. Rank 0 reads the dataset
    // ========================================================

    uint64_t info[2] = {0, 0};

    std::vector<double> all_data;

    if (rank == 0)
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " <dataset_file>" << std::endl;

            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        const std::string filename = argv[1];

        std::ifstream file(filename);

        if (!file)
        {
            std::cerr << "Could not open dataset: " << filename << std::endl;

            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        file >> info[0] >> info[1];

        if (info[0] == 0)
        {
            std::cerr << "Dataset has no dimensions: " << filename << std::endl;

            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        all_data.resize(static_cast<size_t>(info[0] * info[1]));

        for (auto& value : all_data)
        {
            file >> value;
        }
    }

    MPI_Bcast(info, 2, MPI_UINT64_T, 0, MPI_COMM_WORLD);

    const size_t dimensions = static_cast<size_t>(info[0]);
    const size_t num_points = static_cast<size_t>(info[1]);

    // ========================================================
    // 2. Calculate how many points each process gets
    // ========================================================

    const size_t base_points = num_points / world_size;
    const size_t remainder = num_points % world_size;

    const size_t local_points =
        base_points + (static_cast<size_t>(rank) < remainder ? 1 : 0);

    // ========================================================
    // 3. Prepare send counts and displacements
    // ========================================================

    std::vector<int> send_counts(world_size); // Number of values to send to each process
    std::vector<int> displacements(world_size); // Starting index in all_data for each process

    size_t current_offset = 0;

    for (size_t p = 0; p < world_size; ++p)
    {
        const size_t points_for_process =
            base_points + (p < remainder ? 1 : 0);

        send_counts[p] = static_cast<int>(points_for_process * dimensions);

        displacements[p] = static_cast<int>(current_offset * dimensions);

        current_offset += points_for_process;
    }

    // ========================================================
    // 4. Fan-out: rank 0 distributes the data
    // ========================================================

    std::vector<double> local_data(local_points * dimensions);

    MPI_Scatterv(
        all_data.data(),
        send_counts.data(),
        displacements.data(),
        MPI_DOUBLE,

        local_data.data(),
        static_cast<int>(local_data.size()),
        MPI_DOUBLE,

        0,
        MPI_COMM_WORLD
    );

    // ========================================================
    // 5. Local maxima
    // ========================================================

    const std::vector<Item> local_items =
        UnflattenItems(local_data, local_points, dimensions);

    std::vector<Item> local_maxima = BruteForceMaxima(local_items, dimensions);

    // ========================================================
    // 6. Fan-in over a reduction tree
    // ========================================================

    int step = 1;

    while (step < world)
    {
        // ----------------------------------------------------
        // RECEIVERS
        // ----------------------------------------------------

        if (rank % (2 * step) == 0)
        {
            const int sender = rank + step;

            if (sender < world)
            {
                int received_count = 0;

                MPI_Recv(
                    &received_count,
                    1,
                    MPI_INT,
                    sender,
                    0,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );

                std::vector<double> received_data(
                    static_cast<size_t>(received_count) * dimensions
                );

                MPI_Recv(
                    received_data.data(),
                    static_cast<int>(received_data.size()),
                    MPI_DOUBLE,
                    sender,
                    1,
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );

                const std::vector<Item> received_maxima = UnflattenItems(
                    received_data,
                    static_cast<size_t>(received_count),
                    dimensions
                );

                local_maxima = MergeMaxima(local_maxima, received_maxima, dimensions);
            }
        }

        // ----------------------------------------------------
        // SENDERS
        // ----------------------------------------------------

        else
        {
            const int receiver = rank - step;

            const int count = static_cast<int>(local_maxima.size());

            const std::vector<double> maxima_data =
                FlattenItems(local_maxima, dimensions);

            MPI_Request requests[2];

            MPI_Isend(
                &count,
                1,
                MPI_INT,
                receiver,
                0,
                MPI_COMM_WORLD,
                &requests[0]
            );

            MPI_Isend(
                maxima_data.data(),
                static_cast<int>(maxima_data.size()),
                MPI_DOUBLE,
                receiver,
                1,
                MPI_COMM_WORLD,
                &requests[1]
            );

            MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

            break;
        }

        step *= 2;
    }

    if (rank == 0)
    {
        std::cout << "\nNumber of processes: " << world << std::endl;

        std::cout << "\nInitial number of points: " << num_points << std::endl;

        std::cout << "\nFinal maxima: " << local_maxima.size() << std::endl;
    }

    MPI_Finalize();

    return 0;
}
