#pragma once
#include <vector>

namespace utility
{
    template<typename T>
    void eraseSwap(std::vector<T>& vec, size_t index) {
        if (index < vec.size()) {
            std::swap(vec[index], vec.back());
            vec.pop_back();
        }
    }
}
