// Ring (round-robin) method for the maxima problem.
//
// Every process computes the maxima of its share of the data, then tokens of
// candidate points travel once around the ring. A point that survives every
// process is a maximum of the whole set.
//
// Usage: MPI_Ringmethod <dataset_file>

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

// Removes the points of `local` that `incoming` dominates and vice versa.
void CrossFilter(
    std::vector<Item>& local,
    std::vector<Item>& incoming,
    size_t d)
{
    std::vector<bool> local_dominated(local.size(), false);
    std::vector<bool> incoming_dominated(incoming.size(), false);

    for (size_t i = 0; i < local.size(); ++i)
    {
        for (size_t j = 0; j < incoming.size(); ++j)
        {
            if (!incoming_dominated[j] && Dominates(local[i], incoming[j], d))
            {
                incoming_dominated[j] = true;
            }

            if (!local_dominated[i] && Dominates(incoming[j], local[i], d))
            {
                local_dominated[i] = true;
            }
        }
    }

    std::vector<Item> surviving_local;
    surviving_local.reserve(local.size());

    for (size_t i = 0; i < local.size(); ++i)
    {
        if (!local_dominated[i])
        {
            surviving_local.push_back(local[i]);
        }
    }

    std::vector<Item> surviving_incoming;
    surviving_incoming.reserve(incoming.size());

    for (size_t j = 0; j < incoming.size(); ++j)
    {
        if (!incoming_dominated[j])
        {
            surviving_incoming.push_back(incoming[j]);
        }
    }

    local = std::move(surviving_local);
    incoming = std::move(surviving_incoming);
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
    // Calculate how many points each process gets
    // ========================================================

    const size_t base_points = num_points / world_size;
    const size_t remainder = num_points % world_size;

    const size_t local_points =
        base_points + (static_cast<size_t>(rank) < remainder ? 1 : 0);

    // ========================================================
    // Prepare send counts and displacements
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

    std::vector<Item> local_items =
        UnflattenItems(local_data, local_points, dimensions);

    std::vector<Item> local_maxima = BruteForceMaxima(local_items, dimensions);

    // ========================================================
    // ROUND-ROBIN RING
    // ========================================================

    const int next_rank = (rank + 1) % world;
    const int prev_rank = (rank - 1 + world) % world;

    std::vector<Item> outgoing = local_maxima;

    for (int hop = 0; hop < world - 1; ++hop)
    {
        // ----------------------------------------------------
        // Exchange sizes first
        // ----------------------------------------------------

        const int send_count = static_cast<int>(outgoing.size());
        int recv_count = 0;

        MPI_Sendrecv(
            &send_count, 1, MPI_INT, next_rank, 0,
            &recv_count, 1, MPI_INT, prev_rank, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // ----------------------------------------------------
        // Exchange the actual token data
        // ----------------------------------------------------

        const std::vector<double> send_data = FlattenItems(outgoing, dimensions);

        std::vector<double> recv_data(
            static_cast<size_t>(recv_count) * dimensions
        );

        MPI_Sendrecv(
            send_data.data(),
            static_cast<int>(send_data.size()),
            MPI_DOUBLE, next_rank, 1,

            recv_data.data(),
            static_cast<int>(recv_data.size()),
            MPI_DOUBLE, prev_rank, 1,

            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        std::vector<Item> incoming = UnflattenItems(
            recv_data,
            static_cast<size_t>(recv_count),
            dimensions
        );

        // ----------------------------------------------------
        // Decide both directions of domination in one pass,
        // then relay whatever survived incoming onward
        // ----------------------------------------------------

        CrossFilter(local_maxima, incoming, dimensions);

        outgoing = incoming;
    }

    std::vector<double> local_flat = FlattenItems(local_maxima, dimensions);

    const int local_value_count = static_cast<int>(local_flat.size());

    std::vector<int> gather_counts(world_size);

    MPI_Gather(
        &local_value_count, 1, MPI_INT,
        gather_counts.data(), 1, MPI_INT,
        0, MPI_COMM_WORLD
    );

    std::vector<int> gather_displacements(world_size);
    std::vector<double> gathered_data;

    if (rank == 0)
    {
        gather_displacements[0] = 0;

        for (size_t p = 1; p < world_size; ++p)
        {
            gather_displacements[p] =
                gather_displacements[p - 1] + gather_counts[p - 1];
        }

        gathered_data.resize(
            static_cast<size_t>(gather_displacements[world_size - 1]) +
            static_cast<size_t>(gather_counts[world_size - 1])
        );
    }

    MPI_Gatherv(
        local_flat.data(), local_value_count, MPI_DOUBLE,

        gathered_data.data(),
        gather_counts.data(),
        gather_displacements.data(),
        MPI_DOUBLE,

        0, MPI_COMM_WORLD
    );

    if (rank == 0)
    {
        const size_t total_maxima_points = gathered_data.size() / dimensions;

        std::cout << "\nNumber of processes: " << world << std::endl;

        std::cout << "\nInitial number of points: " << num_points << std::endl;

        std::cout << "\nFinal maxima: " << total_maxima_points << std::endl;
    }

    MPI_Finalize();

    return 0;
}
