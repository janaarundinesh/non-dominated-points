#include "knapsack.hpp"


std::vector<Item> ZeroOneKS(std::vector<Item> items)
{
    // 0/1 Knapsack:
    std::vector<Item> NonDominated;

    for (int i = 0 ; i < items.size() ; i++)
    {
        if(dominates(items[i], items[i+1]))
        {
            NonDominated.push_back(items[i]);
        }
        else{
            NonDominated.push_back(items[i+1]);
        }
    }
    return NonDominated;
}
