#include<iostream>
#include "generate_data.hpp"
#include "dominance.hpp"
#include "knapsack.hpp"
#include "sorting.hpp"
#include "maxprofitseen.hpp"

using namespace std;

int main(){
    //Generates Items as a vector ({w1,p1},..)
    vector<Item> items = generateData(10);

    /* 0/1 Knapsack: conventional method */ 

    // items = ZeroOneKS(items);

    //Print: Generated Items
    cout << "Generated Items\n\n";
    for (const auto& item : items)
    {
        cout << "Weight : " << item.weight 
            << ", Profit : " << item.profit 
            << "\n";
    }

    /* 0/1 Knapsack: Sweep and Filter method */

    // Sort items Accorrding to Weight:
    Asc_Sort(items);

    //Max Profit Seen method:
    items = MaxProfitSeen(items);

    /* Results Printing */

    cout << "\nPareto Front (Sweep Filter)\n";

    for (const auto& item : items)
    {
        cout << "Weight : " << item.weight 
            << ", Profit : " << item.profit 
            << "\n";
    }

    return 0;
}

