#include<iostream>
#include "generate_data.hpp"
#include "dominance.hpp"
#include "knapsack.hpp"
#include "sorting.hpp"

using namespace std;

int main(){
    cout << "Pareto Knapsack Thesis\n";

    //Generates Items as a vector ({w1,p1},..)
    vector<Item> items = generateData(10);\

    // Sort items Accorrding to Weight
    items = Asc_Sort(items);

    //Item Print
    for (const auto& item : items)
    {
        cout << "Weight : " << item.weight 
            << ", Profit : " << item.profit 
            << "\n";
    }
    cout << "\n";

    //Compare each item against every other item O(n^2)
    // for (int i = 0; i < items.size(); i++)
    // {
    //     for (int j = 0; j < items.size(); j++)
    //     {
    //         if(i == j)
    //         {
    //             continue;
    //         }
    //         bool dominance = dominates(
//         items[i], items[j]);
//     }

    // 0/1 Knapsack:

    items = ZeroOneKS(items);

    for (const auto& item : items)
    {
        cout << "Weight : " << item.weight 
            << ", Profit : " << item.profit 
            << "\n";
    }

    return 0;
}

