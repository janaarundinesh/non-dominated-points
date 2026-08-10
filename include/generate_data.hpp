#pragma once

#include <vector>
#include <string>
#include "item.hpp"

std::vector<Item> generateData(size_t n, size_t D);

void saveData(const std::vector<Item>& items, const std::string& filename);