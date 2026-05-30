#include<iostream>
#include "generate_data.hpp"
#include "dominance.hpp"

using namespace std;

int main(){
    cout << "Pareto Knapsack Thesis\n";

    vector<Item> items = generateData();

    for (const auto& item : items)
    {
        cout << "Weight : " << item.weight 
            << ", Profit : " << item.profit 
            << "\n";
    }


    return 0;
}