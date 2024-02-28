#pragma once
#include <algorithm>
#include <cstdint>

namespace utility{

    struct SubUpdate {

        SubUpdate() : From(0), End(0) {}
        SubUpdate(size_t from, size_t end) : From(from), End(end) {}

        size_t From;
        size_t End;

        bool Intersects(const SubUpdate& other)const {
            return !(End < other.From || other.End < From);
        }
        void Merge(SubUpdate& other, size_t threshold = 0) {
            if (From > other.From - threshold) {
                From = std::min(other.From, From);
            }
            if (End < other.End + threshold) {
                End = std::max(other.End, End);
            }
        }


    };
}