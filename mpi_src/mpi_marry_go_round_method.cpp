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


namespace {

// Summary file consumed by plots/plot_mpi_results.py. It is appended to, once
// per repetition, and the plotting script takes the median over repetitions.
const char* const kSummaryFile = "../results/mpi_marry_go_round_benchmark.csv";

// Detailed breakdown. One row per run, like the summary file; the plotting
// script reduces the repetitions to a median and keeps the spread.
const char* const kTimingFile = "../results/mpi_marry_go_round_timing.csv";

const char* const kTimingHeader =
    "dimensions,num_points,num_processes,total_s,local_s,crossfilter_s,"
    "communication_s,max_communication_s,barrier_s,scatter_s,gather_s,"
    "load_s,bytes_exchanged,maxima,local_imbalance";

// Appends one measurement to the summary CSV, writing the header only when the
// file is created. Used by both the single-process and the multi-process exit
// path.
void WriteBenchmarkRow(
    uint64_t dimensions,
    uint64_t num_points,
    int processes,
    double execution_time,
    uint64_t maxima)
{
    const std::string output_file = kSummaryFile;

    const bool file_exists = std::ifstream(output_file).good();

    std::ofstream out(output_file, std::ios::app);

    if (!out)
    {
        std::cerr << "Could not open benchmark file: "
                  << output_file << std::endl;

        return;
    }

    if (!file_exists)
    {
        out << "dimensions,num_points,num_processes,"
            << "execution_time,num_non_dominated_points\n";
    }

    out << dimensions << ","
        << num_points << ","
        << processes << ","
        << execution_time << "," // seconds
        << maxima << '\n';
}

// Appends one row of the timing breakdown, writing the header only when the
// file is created.
void WriteTimingRow(
    uint64_t dimensions,
    uint64_t num_points,
    int processes,
    double total_time,
    double local_time,
    double crossfilter_time,
    double communication_time,
    double max_communication_time,
    double barrier_time,
    double scatter_time,
    double gather_time,
    double load_time,
    uint64_t bytes_exchanged,
    uint64_t maxima,
    double local_imbalance)
{
    const std::string output_file = kTimingFile;

    const bool file_exists = std::ifstream(output_file).good();

    std::ofstream out(output_file, std::ios::app);

    if (!out)
    {
        std::cerr << "Could not open timing file: "
                  << output_file << std::endl;

        return;
    }

    if (!file_exists)
    {
        out << kTimingHeader << '\n';
    }

    out << dimensions << ','
        << num_points << ','
        << processes << ','
        << total_time << ','
        << local_time << ','
        << crossfilter_time << ','
        << communication_time << ','
        << max_communication_time << ','
        << barrier_time << ','
        << scatter_time << ','
        << gather_time << ','
        << load_time << ','
        << bytes_exchanged << ','
        << maxima << ','
        << local_imbalance << '\n';
}

} // namespace

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

    // Reading happens before any timing of interest starts, but it is measured
    // so that a run on a cold file system cannot be mistaken for slow
    // communication or a slow algorithm.
    double load_time = 0.0;

    if (rank == 0)
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " <dataset_file>" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        const double load_start = MPI_Wtime();

        std::string filename = argv[1];

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

        load_time = MPI_Wtime() - load_start;
    }

    MPI_Bcast(info,2,MPI_UINT64_T,0,MPI_COMM_WORLD);

    dimensions = info[0];
    num_points = info[1];

    // ========================================================
    // Split data among all processes
    // ========================================================

    uint64_t base_points = num_points / static_cast<uint64_t>(size);
    uint64_t remainder = num_points % static_cast<uint64_t>(size);
    uint64_t local_points = base_points; // Number of points assigned to this process

    if (static_cast<uint64_t>(rank) < remainder)
    {
        local_points++;
    }

    std::vector<double> local_data(local_points * dimensions); // Vector to hold the local data for this process

    // ========================================================
    // Prepare counts and displacements for MPI_Scatterv
    // ========================================================

    const size_t world_size = static_cast<size_t>(size);

    std::vector<int> send_counts(world_size);
    std::vector<int> displacements(world_size);

    uint64_t current_offset = 0;

    for (size_t p = 0; p < world_size; ++p)
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

    // Distribute the data using MPI_Scatterv. Timed with a barrier on both
    // sides, so the number includes waiting for the slowest rank: what the
    // caller actually waits for.

    MPI_Barrier(MPI_COMM_WORLD);
    const double scatter_start = MPI_Wtime();

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

    MPI_Barrier(MPI_COMM_WORLD);
    const double scatter_time = MPI_Wtime() - scatter_start;

    // Start of the region reported as the total: everything the solver does
    // after it has its share of the input, up to the final rotation.
    const double start_time = MPI_Wtime();

    const double local_start = MPI_Wtime();

    std::vector<double> local_maxima = BruteForceMaxima(local_data,local_points,dimensions);

    const double local_time = MPI_Wtime() - local_start;

    // Number of local maxima
    uint64_t local_maxima_points = local_maxima.size() / dimensions;

    if (size == 1)
    {
        const double total_time = MPI_Wtime() - start_time;

        if (rank == 0)
        {
            WriteBenchmarkRow(
                dimensions, num_points, size, total_time, local_maxima_points);

            WriteTimingRow(
                dimensions, num_points, size,
                total_time, local_time,
                0.0, 0.0, 0.0, 0.0,
                scatter_time, 0.0, load_time,
                0,
                local_maxima_points,
                1.0);

            // Same summary line as the multi-process path, so a run prints its
            // result regardless of the process count. One process has no cross
            // filter, no exchange and no barrier to report.
            std::cout << "d=" << dimensions
                      << " N=" << num_points
                      << " P=" << size
                      << " total=" << total_time
                      << " local=" << local_time
                      << " cross=0"
                      << " comm=0"
                      << " barrier=0"
                      << " scatter=" << scatter_time
                      << " gather=0"
                      << " imbalance=1"
                      << " maxima=" << local_maxima_points
                      << std::endl;
        }

        MPI_Finalize();
        return 0;
    }

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

    // The timed region is split into the components a reader would ask about:
    // local work, cross-filter arithmetic, message exchange, and the barrier
    // that closes each round. What is left over is imbalance between ranks.
    double communication_time = 0.0;
    double crossfilter_time = 0.0;
    double barrier_time = 0.0;
    uint64_t bytes_exchanged = 0;

    for (int round = 2; round <= 2 * size - 1; ++round)
    {
        std::vector<double> old_first = first_half;
        std::vector<double> old_second = second_half;

        const double communication_start = MPI_Wtime();

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

        communication_time += MPI_Wtime() - communication_start;

        // Payload actually moved by this rank in this round, both directions,
        // including the two uint64_t count messages.
        bytes_exchanged +=
            (static_cast<uint64_t>(up_payload.size()) +
             static_cast<uint64_t>(incoming_first.size()) +
             static_cast<uint64_t>(old_second.size()) +
             static_cast<uint64_t>(incoming_second.size())) * sizeof(double) +
            4 * sizeof(uint64_t);

        if (rank != size - 1)
        {
            second_half = std::move(incoming_second);
        }
        else
        {
            second_half = std::move(old_first);
        }

        // Do the cross-filtering between first_half and second_half
        const double crossfilter_start = MPI_Wtime();

        auto filtered = CrossFilter(first_half,second_half,dimensions);

        crossfilter_time += MPI_Wtime() - crossfilter_start;

        first_half = std::move(filtered.first);
        second_half = std::move(filtered.second);

        const double barrier_start = MPI_Wtime();

        MPI_Barrier(MPI_COMM_WORLD);

        barrier_time += MPI_Wtime() - barrier_start;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    const double elapsed_time = MPI_Wtime() - start_time;

    std::vector<double> local_result = first_half;

    local_result.insert(local_result.end(),second_half.begin(),second_half.end());

    uint64_t local_result_points =local_result.size() / dimensions;

    // Timed around the two collectives, again with a barrier on both sides.
    MPI_Barrier(MPI_COMM_WORLD);
    const double gather_start = MPI_Wtime();

    std::vector<uint64_t> result_points(world_size);

    MPI_Gather(&local_result_points,1,MPI_UINT64_T,result_points.data(),1,MPI_UINT64_T,0,
        MPI_COMM_WORLD
    );

    std::vector<int> recv_counts(world_size);
    std::vector<int> recv_displacements(world_size);

    uint64_t total_non_dominated = 0;

    if (rank == 0)
    {
        uint64_t gather_offset = 0;

        for (size_t p = 0; p < world_size; ++p)
        {
            recv_counts[p] = static_cast<int>(result_points[p] * dimensions);

            recv_displacements[p] = static_cast<int>(gather_offset * dimensions);

            gather_offset += result_points[p];
        }

        total_non_dominated = gather_offset;
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

    MPI_Barrier(MPI_COMM_WORLD);
    const double gather_time = MPI_Wtime() - gather_start;

    // The breakdown is reported for the rank that determined the wall time:
    // its own components add up to the total, which is what makes the split
    // meaningful. (Taking the maximum of each component separately, as one
    // might expect, lets the parts exceed the whole, because different ranks
    // can be slow in different phases.)
    double max_total = 0.0;

    MPI_Allreduce(&elapsed_time, &max_total, 1, MPI_DOUBLE, MPI_MAX,
        MPI_COMM_WORLD);

    const int not_critical = size;
    const int candidate = (elapsed_time >= max_total) ? rank : not_critical;
    int critical_rank = 0;

    MPI_Allreduce(&candidate, &critical_rank, 1, MPI_INT, MPI_MIN,
        MPI_COMM_WORLD);

    double critical[7] = {
        elapsed_time,
        local_time,
        crossfilter_time,
        communication_time,
        barrier_time,
        scatter_time,
        gather_time
    };

    MPI_Bcast(critical, 7, MPI_DOUBLE, critical_rank, MPI_COMM_WORLD);

    // Reading only happens on rank 0, so the load time is taken from there
    // rather than from the critical rank. The imbalance ratio is the slowest
    // rank's local phase over the mean one; 1.0 means every rank got the same
    // amount of work.
    double local_sum = 0.0;
    double max_local = 0.0;
    double max_load = 0.0;
    double max_communication = 0.0;
    uint64_t max_bytes = 0;

    MPI_Reduce(&local_time, &local_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
        MPI_COMM_WORLD);
    MPI_Reduce(&local_time, &max_local, 1, MPI_DOUBLE, MPI_MAX, 0,
        MPI_COMM_WORLD);
    MPI_Reduce(&load_time, &max_load, 1, MPI_DOUBLE, MPI_MAX, 0,
        MPI_COMM_WORLD);
    MPI_Reduce(&communication_time, &max_communication, 1, MPI_DOUBLE,
        MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&bytes_exchanged, &max_bytes, 1, MPI_UINT64_T, MPI_MAX, 0,
        MPI_COMM_WORLD);

    if (rank == 0)
    {
        const double local_mean = local_sum / static_cast<double>(size);

        const double imbalance =
            local_mean > 0.0 ? max_local / local_mean : 1.0;

        WriteBenchmarkRow(
            dimensions, num_points, size, max_total, total_non_dominated);

        WriteTimingRow(
            dimensions, num_points, size,
            max_total, critical[1], critical[2], critical[3],
            max_communication, critical[4],
            critical[5], critical[6], max_load,
            max_bytes,
            total_non_dominated,
            imbalance);

        std::cout << "d=" << dimensions
                  << " N=" << num_points
                  << " P=" << size
                  << " total=" << max_total
                  << " local=" << critical[1]
                  << " cross=" << critical[2]
                  << " comm=" << critical[3]
                  << " barrier=" << critical[4]
                  << " scatter=" << critical[5]
                  << " gather=" << critical[6]
                  << " imbalance=" << imbalance
                  << " maxima=" << total_non_dominated
                  << std::endl;
    }

    MPI_Finalize();

    return 0;
}
