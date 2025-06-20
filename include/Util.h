#pragma once
#include <string>
#include <vector>

namespace util
{
    std::vector<std::string> stringSplit(const std::string& str, const char* delim);

    template <typename T> 
    void pointerVectorClear(std::vector<T>& vec)
    {
        while (!vec.empty())
        {
            T elem = vec.back();
            delete elem;
            vec.pop_back();
        }
    }
}