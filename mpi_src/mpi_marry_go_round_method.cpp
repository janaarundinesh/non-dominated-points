#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <cstdint>
#include <mpi.h>

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

std::pair<std::vector<double>, std::vector<double>> CrossFilter(
    const std::vector<double>& first_half,
    const std::vector<double>& second_half,
    uint64_t dimensions)
{
    uint64_t first_points =
        first_half.size() / dimensions;

    uint64_t second_points =
        second_half.size() / dimensions;

    std::vector<bool> remove_first(first_points, false);
    std::vector<bool> remove_second(second_points, false);


    // Compare every point in first_half
    // with every point in second_half
    for (uint64_t i = 0; i < first_points; ++i)
    {
        const double* point_first = &first_half[i * dimensions];

        for (uint64_t j = 0; j < second_points; ++j)
        {
            const double* point_second = &second_half[j * dimensions];

            int result = Dominates(point_first, point_second, dimensions);

            if (result == 1)
            {
                // First-half point dominates second-half point
                remove_second[j] = true;
            }
            else if (result == -1)
            {
                // Second-half point dominates first-half point
                remove_first[i] = true;
            }
        }
    }


    // Build filtered first_half
    std::vector<double> filtered_first;

    for (uint64_t i = 0; i < first_points; ++i)
    {
        if (!remove_first[i])
        {
            filtered_first.insert(
                filtered_first.end(),
                &first_half.data()[i * dimensions],
                &first_half.data()[(i + 1) * dimensions]
            );
        }
    }


    // Build filtered second_half
    std::vector<double> filtered_second;

    for (uint64_t j = 0; j < second_points; ++j)
    {
        if (!remove_second[j])
        {
            filtered_second.insert(
                filtered_second.end(),
                &second_half.data()[j * dimensions],
                &second_half.data()[(j + 1) * dimensions]
            );
        }
    }


    return {filtered_first, filtered_second};
}



int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    uint64_t dimensions = 0;
    uint64_t num_points = 0;
    uint64_t info[2] = {0, 0};

    std::vector<double> all_data;

    if (rank == 0)
    {
        std::string filename = "../DataSets/10D_Data/Test6.txt"; // Specify the dataset file path

        std::ifstream file(filename);

        if (!file)
        {
            std::cerr << "Could not open dataset: "
                      << filename << std::endl;

            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        file >> dimensions >> num_points; // Read dimensions and number of points

        info[0] = dimensions;
        info[1] = num_points;
 
        all_data.resize(num_points * dimensions); // Resize the vector to hold all data points

        // Read the data points from the file

        for (uint64_t i = 0; i < num_points; ++i) 
        {
            for (uint64_t j = 0; j < dimensions; ++j)
            {
                file >> all_data[i * dimensions + j];
            }
        }

        file.close();
    }

    MPI_Bcast(info,2,MPI_UINT64_T,0,MPI_COMM_WORLD);

    dimensions = info[0];
    num_points = info[1];

    // ========================================================
    // Split data among all processes
    // ========================================================

    uint64_t base_points = num_points / size;
    uint64_t remainder = num_points % size;
    uint64_t local_points = base_points; // Number of points assigned to this process

    if (static_cast<uint64_t>(rank) < remainder)
    {
        local_points++;
    }

    std::vector<double> local_data(local_points * dimensions); // Vector to hold the local data for this process

    // ========================================================
    // Prepare counts and displacements for MPI_Scatterv
    // ========================================================

    std::vector<int> send_counts(size);
    std::vector<int> displacements(size);

    uint64_t current_offset = 0;

    for (int p = 0; p < size; ++p)
    {
        uint64_t points_for_process = base_points;

        if (static_cast<uint64_t>(p) < remainder)
        {
            points_for_process++;
        }

        // MPI counts/displacements are in number of doubles
        send_counts[p] = static_cast<int>(points_for_process * dimensions);

        displacements[p] = static_cast<int>(current_offset * dimensions);

        current_offset += points_for_process;
    }

    // Distribute the data using MPI_Scatterv

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

    // calculate the time taken for the non-dominated points calculation
    MPI_Barrier(MPI_COMM_WORLD); 
    double start_time = MPI_Wtime();

    std::vector<double> local_maxima = BruteForceMaxima(local_data,local_points,dimensions);


    // Number of local maxima
    uint64_t local_maxima_points = local_maxima.size() / dimensions;

    uint64_t first_half_points = local_maxima_points / 2;

    std::vector<double> first_half(
        local_maxima.data(),
        local_maxima.data() + first_half_points * dimensions
    );

    std::vector<double> second_half(
        local_maxima.data() + first_half_points * dimensions,
        local_maxima.data() + local_maxima.size()
    );

    // ========================================================
    // Merry-go-round rotation
    // ========================================================

    for (int round = 2; round <= 2 * size - 1; ++round)
    {
        std::vector<double> old_first = first_half;
        std::vector<double> old_second = second_half;

        int up_send_to = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;
        int up_receive_from = (rank > 0) ? rank - 1 : MPI_PROC_NULL;

        // Rank 0 sends its bottom player up to rank 1.
        // Every other rank sends its top player to the right.
        const std::vector<double>& up_payload = (rank == 0) ? old_second : old_first;

        uint64_t send_points = up_payload.size() / dimensions;
        uint64_t receive_points = 0;


        // Exchange the number of points first
        MPI_Sendrecv(&send_points,1,MPI_UINT64_T,up_send_to,0,&receive_points,1,MPI_UINT64_T,up_receive_from,0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );

        std::vector<double> incoming_first(receive_points * dimensions);

        // Exchange the actual data
        MPI_Sendrecv(
            up_payload.data(),
            static_cast<int>(up_payload.size()),
            MPI_DOUBLE,
            up_send_to,
            1,

            incoming_first.data(),
            static_cast<int>(incoming_first.size()),
            MPI_DOUBLE,
            up_receive_from,
            1,

            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );


        // Rank 0 keeps its first half fixed.
        if (rank != 0)
        {
            first_half = std::move(incoming_first);
        }

        int down_send_to = (rank > 0) ? rank - 1 : MPI_PROC_NULL;
        int down_receive_from = (rank < size - 1) ? rank + 1 : MPI_PROC_NULL;


        send_points = old_second.size() / dimensions;
        receive_points = 0;


        // Exchange the number of points
        MPI_Sendrecv(&send_points,1,MPI_UINT64_T,down_send_to,2,&receive_points,1,MPI_UINT64_T,down_receive_from,2,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );


        std::vector<double> incoming_second(receive_points * dimensions);

        // Exchange the actual data
        MPI_Sendrecv(
            old_second.data(),
            static_cast<int>(old_second.size()),
            MPI_DOUBLE,
            down_send_to,
            3,

            incoming_second.data(),
            static_cast<int>(incoming_second.size()),
            MPI_DOUBLE,
            down_receive_from,
            3,

            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );


        if (rank != size - 1)
        {
            second_half = std::move(incoming_second);
        }
        else
        {
            second_half = std::move(old_first);
        }

        // Do the cross-filtering between first_half and second_half
        auto filtered = CrossFilter(first_half,second_half,dimensions);

        first_half = std::move(filtered.first);
        second_half = std::move(filtered.second);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    double total_time = 0.0;

    MPI_Reduce(&elapsed_time,&total_time,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    std::vector<double> local_result = first_half;

    local_result.insert(local_result.end(),second_half.begin(),second_half.end());

    uint64_t local_result_points =local_result.size() / dimensions;

    std::vector<uint64_t> result_points(size);

    MPI_Gather(&local_result_points,1,MPI_UINT64_T,result_points.data(),1,MPI_UINT64_T,0,
        MPI_COMM_WORLD
    );

    std::vector<int> recv_counts(size);
    std::vector<int> recv_displacements(size);

    uint64_t total_non_dominated = 0;

    if (rank == 0)
    {
        uint64_t current_offset = 0;

        for (int p = 0; p < size; ++p)
        {
            recv_counts[p] = static_cast<int>(result_points[p] * dimensions);

            recv_displacements[p] = static_cast<int>(current_offset * dimensions);

            current_offset += result_points[p];
        }

        total_non_dominated = current_offset;
    }

    std::vector<double> final_result;

    if (rank == 0)
    {
        final_result.resize(total_non_dominated * dimensions);
    }

    MPI_Gatherv(
    local_result.data(),
    static_cast<int>(local_result.size()),
    MPI_DOUBLE,

    final_result.data(),
    recv_counts.data(),
    recv_displacements.data(),
    MPI_DOUBLE,

    0,
    MPI_COMM_WORLD
    );

    if (rank == 0)
    {
        std::cout << "Number of processors: " << size << std::endl;
        std::cout << "Elapsed time : " << total_time << " seconds" << std::endl;
        std::cout << "Dimensions: " << dimensions << std::endl;
        std::cout << "Number of points: " << num_points << std::endl;
        std::cout << "Number of non-dominated points: " << total_non_dominated << std::endl;
    }

    MPI_Finalize();

    return 0;
}