#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <mpi.h>

#include "brute_force_maxima.hpp"
#include "dominance.hpp"


std::vector<double> FlattenItems(
    const std::vector<Item>& items,
    size_t dimensions)
{
    std::vector<double> data(
        items.size() * dimensions
    );

    for (size_t i = 0; i < items.size(); ++i)
    {
        for (size_t j = 0; j < dimensions; ++j)
        {
            data[i * dimensions + j] =
                items[i].coords[j];
        }
    }

    return data;
}



std::vector<Item> UnflattenItems(
    const std::vector<double>& data,
    size_t number_of_items,
    size_t dimensions)
{
    std::vector<Item> items(number_of_items);

    for (size_t i = 0; i < number_of_items; ++i)
    {
        items[i].coords.resize(dimensions);

        for (size_t j = 0; j < dimensions; ++j)
        {
            items[i].coords[j] =
                data[i * dimensions + j];
        }
    }

    return items;
}

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


int main(int argc, char** argv)
{
    int rank;
    int size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    uint64_t dimensions = 0;
    uint64_t num_points = 0;

    uint64_t info[2] = {0, 0};

    std::vector<double> all_data;

    if (rank == 0)
    {
        std::string filename = "../DataSets/10D_Data/Test5.txt";

        std::ifstream file(filename);

        if (!file)
        {
            std::cerr << "Could not open dataset: " << filename << std::endl;

            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        file >> dimensions >> num_points;

        all_data.resize(num_points * dimensions);

        for (uint64_t i = 0; i < num_points; ++i)
        {
            for (uint64_t j = 0; j < dimensions; ++j)
            {
                file >> all_data[
                    i * dimensions + j
                ];
            }
        }

        file.close();

        info[0] = dimensions;
        info[1] = num_points;
    }

    MPI_Bcast(info, 2, MPI_UINT64_T, 0, MPI_COMM_WORLD);

    dimensions = info[0];
    num_points = info[1];


    // ========================================================
    // Calculate how many points each process gets
    // ========================================================

    uint64_t base_points = num_points / size;

    uint64_t remainder = num_points % size;

    uint64_t local_points = base_points;

    if (static_cast<uint64_t>(rank) < remainder)
    {
        local_points++;
    }


    // ========================================================
    // Prepare send counts and displacements
    // ========================================================

    std::vector<int> send_counts(size); // Number of values from the data to send to each process
    std::vector<int> displacements(size); // Starting index in the all_data array for each process

    uint64_t current_offset = 0;

    for (int p = 0; p < size; ++p)
    {
        uint64_t points_for_process = base_points;

        if (static_cast<uint64_t>(p) < remainder)
        {
            points_for_process++;
        }

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
        UnflattenItems(
            local_data,
            local_points,
            dimensions
        );


    std::vector<Item> local_maxima = BruteForceMaxima(local_items, dimensions);


    // ========================================================
    // ROUND-ROBIN RING
    // ========================================================

    int next_rank = (rank + 1) % size;
    int prev_rank = (rank - 1 + size) % size;

    std::vector<Item> outgoing = local_maxima;

    for (int hop = 0; hop < size - 1; ++hop)
    {
        // ----------------------------------------------------
        // Exchange sizes first
        // ----------------------------------------------------

        int send_count = static_cast<int>(outgoing.size());
        int recv_count = 0;

        MPI_Sendrecv(
            &send_count, 1, MPI_INT, next_rank, 0,
            &recv_count, 1, MPI_INT, prev_rank, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // ----------------------------------------------------
        // Exchange the actual token data
        // ----------------------------------------------------

        std::vector<double> send_data = FlattenItems(outgoing, dimensions);
        std::vector<double> recv_data(static_cast<size_t>(recv_count) * dimensions);

        MPI_Sendrecv(
            send_data.data(),
            static_cast<int>(send_data.size()),
            MPI_DOUBLE, next_rank, 1,

            recv_data.data(),
            static_cast<int>(recv_data.size()),
            MPI_DOUBLE, prev_rank, 1,

            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        std::vector<Item> incoming = UnflattenItems(recv_data, recv_count, dimensions);

        // ----------------------------------------------------
        // Decide both directions of domination in one pass,
        // then relay whatever survived incoming onward
        // ----------------------------------------------------

        CrossFilter(local_maxima, incoming, dimensions);

        outgoing = incoming;
    }

    int local_count = static_cast<int>(local_maxima.size());
    std::vector<double> local_flat = FlattenItems(local_maxima, dimensions);
    int local_value_count = static_cast<int>(local_flat.size());

    std::vector<int> gather_counts(size);

    MPI_Gather(
        &local_value_count, 1, MPI_INT,
        gather_counts.data(), 1, MPI_INT,
        0, MPI_COMM_WORLD
    );

    std::vector<int> gather_displacements(size);
    std::vector<double> gathered_data;

    if (rank == 0)
    {
        gather_displacements[0] = 0;

        for (int p = 1; p < size; ++p)
        {
            gather_displacements[p] =
                gather_displacements[p - 1] + gather_counts[p - 1];
        }

        int total_values =
            gather_displacements[size - 1] + gather_counts[size - 1];

        gathered_data.resize(total_values);
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
        uint64_t total_maxima_points = static_cast<uint64_t>(gathered_data.size()) / dimensions;

        std::cout << "\nNumber of processes: " << size << std::endl;

        std::cout << "\nInitial number of points: " << num_points << std::endl;

        std::cout << "\nFinal maxima: " << total_maxima_points << std::endl;
    }

    MPI_Finalize();

    return 0;
}