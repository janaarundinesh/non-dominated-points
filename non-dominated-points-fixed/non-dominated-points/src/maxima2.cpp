#include "maxima2.hpp"
#include "sorting.hpp"
#include "filter.hpp"
#include "dominance.hpp"

// O(n^2) fallback: keeps a point iff no other point in the same set
// dominates it on the first d coordinates. Used only when the
// recursive split below can't make sound progress (every point tied
// on the split coordinate).
static std::vector<Item> BruteForceMaxima(
    const std::vector<Item>& points,
    size_t d
)
{
    std::vector<Item> result;

    for(size_t i = 0; i < points.size(); i++)
    {
        bool dominated = false;

        for(size_t j = 0; j < points.size(); j++)
        {
            if(i == j) continue;

            if(Dominates(points[j], points[i], d))
            {
                dominated = true;
                break;
            }
        }

        if(!dominated)
        {
            result.push_back(points[i]);
        }
    }

    return result;
}

std::vector<Item> MAXIMA2(
    std::vector<Item> points,
    size_t d
)
{
    if(points.size() <= 1)
    {
        return points;
    }

    // Split on the d-th active coordinate (coords[d-1]) so that every
    // point in S1 has a coordinate-d value strictly less than every
    // point in S2. That strict separation is what lets the merge step
    // below skip re-checking coordinate d: a point of M1 can only be
    // dominated by M2 via the remaining d-1 coordinates, and a point
    // of M1 can never dominate a point of M2 (it's already behind on
    // coordinate d).
    //
    // A plain positional median isn't safe on its own: if several
    // points share the value at the cut, naively splitting there can
    // put tied points on both sides, and a "tied on coord d" point in
    // S1 can still dominate one in S2 via the other coordinates. So we
    // push the cut past any ties on the split value.
    SortByCoordinate(points, d - 1);

    size_t mid = points.size()/2;

    while(mid < points.size() &&
          points[mid].coords[d-1] == points[mid-1].coords[d-1])
    {
        mid++;
    }

    if(mid == points.size())
    {
        // The tie run reaches the end: every point shares the same
        // value on this axis (or at least everything past the median
        // does, with no way to leave even one point strictly behind).
        // The axis carries no usable separating information here, so
        // resolve this set directly rather than splitting unsoundly
        // or recursing on an unchanged S1 forever.
        return BruteForceMaxima(points, d);
    }

    std::vector<Item> S1(
        points.begin(),
        points.begin()+mid
    );

    std::vector<Item> S2(
        points.begin()+mid,
        points.end()
    );

    auto M1 = MAXIMA2(S1,d);
    auto M2 = MAXIMA2(S2,d);

    if(d == 1)
    {
        // S1's coord-0 values are strictly less than S2's (guaranteed
        // by the tie-free split above), so the unique maximum lives
        // entirely within M2.
        return M2;
    }

    // Every point in M1 has coord-d strictly less than every point in
    // M2 (guaranteed by the tie-free split above), so that coordinate
    // alone already supplies the "strict" part of domination. A point
    // of M1 is therefore dominated overall iff some point of M2
    // weakly covers it on the remaining d-1 coordinates -- no separate
    // strict difference is needed there. WeakCoverFilter implements
    // exactly that (with the usual exact-duplicate exception, which
    // can't actually arise here since M1/M2 differ on coordinate d-1).
    auto survivors = WeakCoverFilter(M1, M2, d - 1);

    survivors.insert(
        survivors.end(),
        M2.begin(),
        M2.end()
    );

    return survivors;
}
