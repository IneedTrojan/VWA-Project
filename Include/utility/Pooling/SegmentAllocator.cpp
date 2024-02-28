#include "SegmentAllocator.h"  // Assuming this is the header file name.
#include <algorithm>
#include <cassert>
#include <iostream>
#include <exception>
#include "glm/glm.hpp"

namespace utility {

    SegmentAllocator::SegmentAllocator() : totalMemory(0), numSegments(0) {}

    SegmentAllocator::SegmentAllocator(uint32_t size) {
        resize(size, true);
    }

    void SegmentAllocator::resize(size_t size, bool ignoreResize){
        if (ignoreResize||canResize()) {

            locationSorted.clear();
            size_sorted_map.clear();
            memory.clear();

            size -= 1;
            totalMemory = static_cast<uint32_t>(size);
            numSegments = IntLog2(static_cast<uint32_t>(size)) + 1;
            size_sorted_map.resize(numSegments);
            uint32_t memIndex = memory.alloc();
            data(memIndex) = Bounds(0, static_cast<uint32_t>(size));
            View mem = View(memIndex, data(memIndex));
            free(mem);
        }
        else {
            throw std::exception("can't resize a pool while memory is allocated");
        }
    }
    bool SegmentAllocator::canResize(){
        return locationSorted.size() == 1 && data(locationSorted[0]).size() == totalMemory;
    }

    void SegmentAllocator::SplitBlocks(std::vector<View>& bounds, std::vector<View>& splitBounds) {
        for (auto view : bounds) {
            for (uint32_t min = view.min; min < view.max; min++) {
                uint32_t blockIndex = memory.alloc();
                data(blockIndex) = Bounds(min, min + 1);
                splitBounds.emplace_back(blockIndex, data(blockIndex));
            }
            memory.Delete(view.index);
        }
    }

    void SegmentAllocator::MallocContignious(uint32_t size, std::vector<View>& bounds) {

        uint32_t cpy = size;
        while (cpy > 0) {
            View view = MallocBest(cpy);
            cpy -= view.size();
            if (!view.IsValid()) {
                std::cout << "Out of memory MallocContignious";
                break;
            }
            bounds.emplace_back(view);
        }
    }

    SegmentAllocator::View SegmentAllocator::MallocBest(uint32_t size) {

        View view = malloc(size);
        if (!view.IsValid()) {
            for (uint32_t offset = IntLog2(size); offset != -1; offset--) {
                auto& vec = size_sorted_map[offset];
                if (vec.size() != 0) {
                    uint32_t last = vec.back();
                    vec.pop_back();
                    location_remove(last);
                    view = View(last, data(last));
                    break;
                }
            }
        }
        return view;
    }

    SegmentAllocator::View SegmentAllocator::malloc(uint32_t size) {

        for (uint32_t hash = IntLog2(size); hash < numSegments; hash++) {
            auto& vec = size_sorted_map[hash];
            if (vec.size() != 0) {

                auto it = size_findBy(vec, size);
                if (it != vec.end()) {
                    uint32_t blockIndex = *it;
                    Bounds& block = data(blockIndex);

                    size_remove(blockIndex);
                    if (block.size() == size) {

                        location_remove(blockIndex);
                        return View(blockIndex, block);
                    }
                    if (block.size() != size) {

                        uint32_t offset = block.min;
                        block.min += size;
                        size_emplace(blockIndex);

                        uint32_t viewIndex = memory.alloc();
                        data(viewIndex).Reset();
                        data(viewIndex) = Bounds(offset, offset + size);
                        return View(viewIndex, Bounds(offset, offset + size));
                    }
                }
            }
        }
        return View();
    }

    void SegmentAllocator::free(View& view) {
        location_emplace(view.index);
        view.Reset();
    }

    SegmentAllocator::Bounds& SegmentAllocator::data(uint32_t index) {
        return *memory.Get(index);
    }
    std::vector<uint32_t>::iterator SegmentAllocator::location_findBy(std::vector<uint32_t>& vec, uint32_t index) {
        return std::lower_bound(vec.begin(), vec.end(), index, [this](uint32_t estimate, uint32_t goal) {
            return data(estimate).min < data(goal).min;
            }
        );
    }

    std::vector<uint32_t>::iterator SegmentAllocator::size_findBy(std::vector<uint32_t>& vec, uint32_t size) {
        return std::lower_bound(vec.begin(), vec.end(), size, [this](uint32_t a, uint32_t target_size) {
            return data(a).size() < target_size;
            }
        );
    }

    std::vector<uint32_t>::iterator SegmentAllocator::index_size_findBy(std::vector<uint32_t>& vec, uint32_t index) {
        Bounds& bound = data(index);
        return std::lower_bound(vec.begin(), vec.end(), index, [this, &bound](uint32_t a, uint32_t b) {
            if (data(a).size() != bound.size()) return data(a).size() < bound.size();
            return a < b;
            }
        );
    }

    void SegmentAllocator::location_remove(uint32_t index) {
        std::vector<uint32_t>& vec = locationSorted;
        auto it = location_findBy(vec, index);
        if (it != vec.end() && *it == index) {
            vec.erase(it);
        }
    }

    void SegmentAllocator::location_emplace(uint32_t index) {
        Bounds& block = data(index);
        std::vector<uint32_t>& vec = locationSorted;
        auto it = location_findBy(vec, index);

        glm::uvec2 distance = glm::uvec2(
            std::distance(vec.begin(), it) - 1,
            std::distance(vec.begin(), it)
        );
        glm::uvec2 blockIndex = glm::uvec2(
            distance.x != -1 ? vec[distance.x] : -1,
            distance.y != vec.size() ? vec[distance.y] : -1
        );
        Bounds* left = blockIndex.x != -1 ? &data(blockIndex.x) : nullptr;
        Bounds* right = blockIndex.y != -1 ? &data(blockIndex.y) : nullptr;
        const bool useMerge = true;

        bool mergeLeft = false;
        bool mergeRight = false;

        if (left && left->Intersects(block) && useMerge) {
            size_remove(blockIndex.x);
            *left = left->Merge(block);

            mergeLeft = true;
        }
        if (right && right->Intersects(block) && useMerge) {
            size_remove(blockIndex.y);
            *right = right->Merge(block);

            mergeRight = true;
        }
        if (mergeLeft) {

            if (mergeRight) {
                *left = left->Merge(*right);
                locationSorted.erase(it);

                memory.Delete(blockIndex.y);
            }
            size_emplace(blockIndex.x);

        }
        if (!mergeLeft && mergeRight) {
            size_emplace(blockIndex.y);
        }
        if (!mergeLeft && !mergeRight) {
            size_emplace(index);
            vec.insert(it, index);
        }
        if (mergeLeft || mergeRight) {
            memory.Delete(index);
        }
    }

    void SegmentAllocator::size_remove(uint32_t index) {
        Bounds& bound = data(index);
        uint32_t hash = IntLog2(bound.size());
        std::vector<uint32_t>& vec = size_sorted_map[hash];
        auto it = index_size_findBy(vec, index);
        if (it != vec.end() && *it == index) {
            vec.erase(it);
        }
    }

    void SegmentAllocator::size_emplace(uint32_t index) {
        Bounds& bound = data(index);
        uint32_t hash = IntLog2(bound.size());
        std::vector<uint32_t>& vec = size_sorted_map[hash];
        auto it = index_size_findBy(vec, index);
        vec.insert(it, index);
    }
    uint32_t SegmentAllocator::IntLog2(uint32_t size) {
        uint32_t result = 0;
        while (size >= 2) {
            size = size >> 1;
            result++;
        }
        return result;
    }
}
