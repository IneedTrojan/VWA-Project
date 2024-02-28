#pragma once
#include "Pool.h"


namespace utility {


    template<typename T>
    struct Bounds {
        T min;
        T max;

        Bounds(T Min, T Max) :min(Min), max(Max) {

        }

        Bounds() :min(0), max(0) {

        }

        T size() {
            return max - min;
        }
        bool Intersects(Bounds<T> other) {
            return max >= other.min && min <= other.max;
        }
        Bounds<T> Merge(Bounds<T> other) {
            return { std::min(min, other.min), std::max(max, other.max) };
        }

        bool IsValid() {
            return max > min;
        }
        void Reset() {
            min = 0;
            max = 0;
        }
    };

    class SegmentAllocator {
    public:
        using Bounds = Bounds<uint32_t>;
        using IndexPtr = uint32_t;

        struct View : Bounds {
            uint32_t index;
            View() : index(0), Bounds() {}
            View(uint32_t Index, Bounds base) :index(Index), Bounds(base) {

            }

            void Reset() {
                Bounds::Reset();
                index = 0;
            }
        };
    private:
        Pool<Bounds> memory;



        std::vector<std::vector<uint32_t>> size_sorted_map;
        std::vector<uint32_t> locationSorted;

        uint32_t totalMemory;
        uint32_t numSegments;

    public:

        SegmentAllocator();
        SegmentAllocator(uint32_t size);

        void SplitBlocks(std::vector<View>& bounds, std::vector<View>& splitBounds);
        void MallocContignious(uint32_t size, std::vector<View>& bounds);
        View MallocBest(uint32_t size);
        View malloc(uint32_t size);
        void free(View& view);
        void resize(size_t size, bool ignoreResize = false);
        bool canResize();


    private:
        Bounds& data(uint32_t blockIndex);
        std::vector<uint32_t>::iterator location_findBy(std::vector<uint32_t>& vec, uint32_t index);
        std::vector<uint32_t>::iterator size_findBy(std::vector<uint32_t>& vec, uint32_t size);
        std::vector<uint32_t>::iterator index_size_findBy(std::vector<uint32_t>& vec, uint32_t index);

        void location_remove(uint32_t index);
        void location_emplace(uint32_t index);

        void size_remove(uint32_t index);
        void size_emplace(uint32_t index);
        uint32_t IntLog2(uint32_t size);
    };
}
