// Contract of the dataset loader.
//
// Everything downstream assumes that a successfully loaded PointSet is
// well-formed: every point has exactly `dimensions` coordinates and the declared
// number of points is present. The loader is the only place that can guarantee
// it, so its refusal paths are worth a test of their own --- a loader that
// silently returns a half-filled set would turn into out-of-bounds reads much
// later, in code that has no way to detect the problem.

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "dataset.hpp"

namespace fs = std::filesystem;

namespace {

int failures = 0;

void Check(bool condition, const std::string& what)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << what << '\n';

        ++failures;
    }
}

fs::path MakeTemporaryDirectory()
{
    const auto stamp = static_cast<unsigned long long>(
        std::chrono::steady_clock::now().time_since_epoch().count());

    const fs::path directory =
        fs::temp_directory_path() / ("ndp_dataset_test_" + std::to_string(stamp));

    fs::create_directories(directory);

    return directory;
}

fs::path WriteFile(const fs::path& directory, const std::string& name,
                   const std::string& contents)
{
    const fs::path path = directory / name;

    std::ofstream file(path);

    file << contents;

    return path;
}

} // namespace

int main()
{
    const fs::path directory = MakeTemporaryDirectory();

    // A well-formed 3D dataset with two points.
    const fs::path good = WriteFile(
        directory, "good.txt",
        "3 2\n"
        "1 2 3\n"
        "4 5 6\n");

    const auto loaded = LoadPointSet(good.string());

    Check(loaded.has_value(), "a well-formed dataset loads");

    if (loaded)
    {
        Check(loaded->dimensions == 3, "dimension is read from the header");
        Check(loaded->points.size() == 2, "point count is read from the header");
        Check(loaded->points[1].coords.size() == 3,
              "every point has one coordinate per dimension");
        Check(loaded->points[1].coords[2] == 6.0,
              "coordinates are read in row-major order without loss");
    }

    // Missing file.
    Check(!LoadPointSet((directory / "absent.txt").string()).has_value(),
          "a missing file is rejected");

    // Header that is not two numbers.
    const fs::path bad_header = WriteFile(directory, "bad_header.txt",
                                          "dimension points\n1 2 3\n");
    Check(!LoadPointSet(bad_header.string()).has_value(),
          "a non-numeric header is rejected");

    // Zero dimensions would make every coordinate comparison vacuous.
    const fs::path no_dimensions = WriteFile(directory, "no_dimensions.txt",
                                             "0 2\n");
    Check(!LoadPointSet(no_dimensions.string()).has_value(),
          "a zero dimension is rejected");

    // Declares three points, supplies one.
    const fs::path truncated = WriteFile(directory, "truncated.txt",
                                         "2 3\n1 2\n3 4\n");
    Check(!LoadPointSet(truncated.string()).has_value(),
          "a dataset that ends early is rejected");

    // A point with too few coordinates: the file ends inside the last point.
    const fs::path short_point = WriteFile(directory, "short_point.txt",
                                           "3 2\n1 2 3\n4 5\n");
    Check(!LoadPointSet(short_point.string()).has_value(),
          "a point with missing coordinates is rejected");

    // Ordinary formatting variations must still load: extra blank lines,
    // trailing whitespace, and a file that does not end with a newline.
    const fs::path messy = WriteFile(directory, "messy.txt",
                                     "2 2\n\n  1 2\n3 4   ");
    const auto messy_set = LoadPointSet(messy.string());
    Check(messy_set.has_value() && messy_set->points.size() == 2,
          "blank lines and trailing spaces do not change the result");

    fs::remove_all(directory);

    if (failures != 0)
    {
        std::cerr << failures << " loader check(s) failed\n";

        return 1;
    }

    std::cout << "dataset loader: all checks passed\n";

    return 0;
}
