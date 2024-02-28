#pragma once
#include <cstddef>
#include <stdexcept>

namespace utility
{
    template <typename T, std::size_t N>
    class stackAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using void_pointer = void*;
        using const_void_pointer = const void*;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template <class U>
        struct rebind { using other = stackAllocator<U, N>; };

        stackAllocator() noexcept : memory_(), current_(memory_) {}
        template <typename U>
        stackAllocator(const stackAllocator<U, N>&) noexcept {}

        T* allocate(std::size_t n) {
            if (static_cast<size_type>(current_ - memory_) + n > N) {
                throw std::bad_alloc();
            }
            T* result = reinterpret_cast<T*>(current_);
            current_ += n * sizeof(T);
            return result;
        }

        void deallocate(T* p, std::size_t n) noexcept {
            // Stack-like deallocation: only deallocate if it's the last allocated block
            if (reinterpret_cast<char*>(p) + n * sizeof(T) == current_) {
                current_ = reinterpret_cast<char*>(p);
            }
        }

        size_type max_size() const noexcept {
            return N;
        }

    private:
        alignas(T) char memory_[N * sizeof(T)];
        char* current_;
    };

    // Comparison operators
    template <class T1, std::size_t N1, class T2, std::size_t N2>
    bool operator==(const stackAllocator<T1, N1>&, const stackAllocator<T2, N2>&) noexcept {
        return N1 == N2;
    }

    template <class T1, std::size_t N1, class T2, std::size_t N2>
    bool operator!=(const stackAllocator<T1, N1>& a, const stackAllocator<T2, N2>& b) noexcept {
        return !(a == b);
    }
}
