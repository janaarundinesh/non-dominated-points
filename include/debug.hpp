#pragma once

#include <iostream>

// Set to 1 to enable debug output
#define DEBUG_FILTER 0

#if DEBUG_FILTER
    #define DEBUG_PRINT(x) (std::cout << x << std::endl);
#else
    #define DEBUG_PRINT(x)
#endif