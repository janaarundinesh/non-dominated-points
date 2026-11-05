#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

#include "maxima2.hpp"
#include "point_set.hpp"
#include "print.hpp"
#include "validator.hpp"

namespace {

constexpr int kFirstDataset = 1;
constexpr int kLastDataset = 16;

// The maxima of a point set are an unordered set: compare as multisets rather
// than relying on the order in which either side happens to list them.
bool CompareMaxima(const std::vector<Item>& computed, const std::vector<Item>& expected)
{
    if (computed.size() != expected.size())
    {
        return false;
    }

    auto sorted = [](std::vector<Item> items)
    {
        std::sort(items.begin(), items.end(),
            [](const Item& a, const Item& b)
            {
                return a.coords < b.coords;
            }
        );

        return items;
    };

    const std::vector<Item> a = sorted(computed);
    const std::vector<Item> b = sorted(expected);

    return std::equal(a.begin(), a.end(), b.begin(),
        [](const Item& x, const Item& y)
        {
            return x.coords == y.coords;
        }
    );
}

} // namespace

int main()
{
    int passed = 0;
    const int total = kLastDataset - kFirstDataset + 1;

    for (int i = kFirstDataset; i <= kLastDataset; i++)
    {
        const std::string filename =
            "../validation/test_datasets/Test" + std::to_string(i) + ".txt";

        const std::optional<ValidationDataset> dataset = LoadDataset(filename);

        if (!dataset)
        {
            std::cout << "Test Case " << i << " : SKIPPED (could not load "
                 << filename << ")\n\n";

            continue;
        }

        const PointSet set{dataset->dimension, dataset->inputPoints};

        const auto maxima = MAXIMA2(set);

        if (CompareMaxima(maxima, dataset->expectedMaxima))
        {
            std::cout << "Test Case " << i << " : PASSED\n\n";
            passed++;
        }
        else
        {
            std::cout << "Test Case " << i << " : FAILED\n";

            std::cout << "\nExpected Maxima\n";
            printItems(dataset->expectedMaxima);

            std::cout << "\nComputed Maxima\n";
            printItems(maxima);

            std::cout << "\n";
        }
    }

    std::cout << "\n=====================================\n";
    std::cout << "Validation Summary\n";
    std::cout << "=====================================\n";
    std::cout << "Passed : " << passed << " / " << total << "\n";

    return passed == total ? 0 : 1;
}
