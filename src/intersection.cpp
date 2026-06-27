#include "intersection.hpp"

static bool Equal(
    const Item& a,
    const Item& b
)
{
    return a.coords == b.coords;
}

std::vector<Item> Intersect(
    const std::vector<Item>& A,
    const std::vector<Item>& B
)
{
    std::vector<Item> result;

    for(const auto& a : A)
    {
        for(const auto& b : B)
        {
            if(Equal(a,b))
            {
                result.push_back(a);
                break;
            }
        }
    }

    return result;
}