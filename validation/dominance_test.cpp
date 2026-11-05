// The dominance relation on its own.
//
// MAXIMA2 and the brute-force baseline share this predicate, so a differential
// test between them cannot catch a mistake inside it: both would be wrong in the
// same way and would still agree. The tie handling is the part worth pinning
// down. "Strictly better in at least one coordinate and never worse in the
// others" is what makes a point with a tied coordinate fail to dominate its own
// copy, and the whole merge step depends on that.

#include <initializer_list>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "dominance.hpp"
#include "item.hpp"

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

Item Point(std::initializer_list<double> coordinates)
{
    return Item{std::vector<double>(coordinates)};
}

std::mt19937 rng(4242u);

// Small ranges on purpose: ties on individual coordinates are the common case
// here, not the exception.
Item RandomPoint(size_t dimensions)
{
    std::uniform_int_distribution<int> value(0, 2);

    std::vector<double> coordinates(dimensions);

    for (auto& coordinate : coordinates)
    {
        coordinate = static_cast<double>(value(rng));
    }

    return Item{coordinates};
}

} // namespace

int main()
{
    // --- the definition ---------------------------------------------------

    Check(Dominates(Point({2.0, 1.0}), Point({1.0, 1.0}), 2),
          "a strict improvement with a tie elsewhere dominates");

    Check(!Dominates(Point({1.0, 1.0}), Point({1.0, 1.0}), 2),
          "a point does not dominate itself");

    Check(!Dominates(Point({1.0, 1.0}), Point({2.0, 1.0}), 2),
          "the direction of the comparison matters");

    Check(Dominates(Point({2.0, 2.0}), Point({1.0, 1.0}), 2),
          "a strict improvement in every coordinate dominates");

    Check(!Dominates(Point({1.0, 2.0}), Point({2.0, 1.0}), 2),
          "one better and one worse coordinate is incomparable");

    Check(!Dominates(Point({2.0, 1.0}), Point({1.0, 2.0}), 2),
          "the mirrored incomparable pair is incomparable too");

    Check(!Dominates(Point({1.0, 1.0}), Point({1.0, 2.0}), 2),
          "a tie plus a worse coordinate is not dominance");

    // --- dimension handling -----------------------------------------------

    Check(Dominates(Point({2.0, 0.0}), Point({1.0, 9.0}), 1),
          "coordinates at or beyond d are not compared");

    Check(!Dominates(Point({1.0}), Point({1.0}), 1),
          "one-dimensional equality is not dominance");

    Check(Dominates(Point({3.0}), Point({2.0}), 1),
          "one-dimensional improvement is dominance");

    // --- weak dominance ---------------------------------------------------

    Check(WeaklyDominates(Point({1.0, 1.0}), Point({1.0, 1.0}), 2),
          "equality is weak dominance");

    Check(WeaklyDominates(Point({2.0, 1.0}), Point({1.0, 1.0}), 2),
          "a tie elsewhere does not prevent weak dominance");

    Check(!WeaklyDominates(Point({1.0, 1.0}), Point({1.0, 2.0}), 2),
          "a worse coordinate breaks weak dominance");

    Check(!WeaklyDominates(Point({1.0, 1.0}), Point({2.0, 1.0}), 2),
          "weak dominance is also directional");

    // --- properties, on tied random data ----------------------------------

    for (size_t dimensions = 1; dimensions <= 4; ++dimensions)
    {
        std::vector<Item> pool(40);

        for (auto& point : pool)
        {
            point = RandomPoint(dimensions);
        }

        bool antisymmetric = true;
        bool strict_implies_weak = true;

        for (const Item& a : pool)
        {
            for (const Item& b : pool)
            {
                if (Dominates(a, b, dimensions) && Dominates(b, a, dimensions))
                {
                    antisymmetric = false;
                }

                if (Dominates(a, b, dimensions) &&
                    !WeaklyDominates(a, b, dimensions))
                {
                    strict_implies_weak = false;
                }
            }
        }

        const std::string suffix = " (d=" + std::to_string(dimensions) + ")";

        Check(antisymmetric, "two points never dominate each other" + suffix);

        Check(strict_implies_weak,
              "strict dominance implies weak dominance" + suffix);

        // Transitivity, on the same tied pool: with values in {0,1,2} most
        // pairs are ties or incomparable, so a chain a > b > c is a real find.
        bool transitive = true;
        bool weak_transitive = true;

        for (const Item& a : pool)
        {
            for (const Item& b : pool)
            {
                for (const Item& c : pool)
                {
                    if (Dominates(a, b, dimensions) &&
                        Dominates(b, c, dimensions) &&
                        !Dominates(a, c, dimensions))
                    {
                        transitive = false;
                    }

                    if (WeaklyDominates(a, b, dimensions) &&
                        WeaklyDominates(b, c, dimensions) &&
                        !WeaklyDominates(a, c, dimensions))
                    {
                        weak_transitive = false;
                    }
                }
            }
        }

        Check(transitive, "dominance is transitive" + suffix);
        Check(weak_transitive, "weak dominance is transitive" + suffix);
    }

    if (failures != 0)
    {
        std::cerr << failures << " dominance check(s) failed\n";

        return 1;
    }

    std::cout << "dominance relation: all checks passed\n";

    return 0;
}
