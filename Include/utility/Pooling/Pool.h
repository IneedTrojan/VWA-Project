#pragma once
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdint>

namespace utility {
    template<typename ObjectType>
    class Pool
    {
        std::vector<ObjectType> items;
        std::vector<int> stack;
    public:

        Pool() {

        }

        void clear() {
            items.clear();
            stack.clear();
        }

        bool recycle(int& index, ObjectType*& collect)
        {
            if (stack.empty())
            {
                return false;
            }
            index = stack.back();
            stack.pop_back();
            collect = items.data() + index;
            return true;
        }

        void emplace(int& index, const ObjectType& value, ObjectType*& collect)
        {
            index = static_cast<int>(items.size());
            items.resize(index + 1);
            collect = items.data() + index;
            items[index] = value;

        }
        int alloc()
        {
            if (stack.empty())
            {
                items.resize(items.size() + 1);
                return static_cast<int>(items.size() - 1);
            }
            int index = stack.back();
            stack.pop_back();
            return index;
        }



        void Delete(int index)
        {
            if (index < 0 || index >= static_cast<int>(items.size()))
            {
                throw std::out_of_range("Index out of range");
            }
            stack.emplace_back(index);
        }

        ObjectType* Get(int index)
        {
            if (index < 0 || index >= static_cast<int>(items.size()))
            {
                throw std::out_of_range("Index out of range");
            }
            return &items[index];
        }

        Pool(Pool<ObjectType>&& other) noexcept
            : items(std::move(other.items)),
            stack(std::move(other.stack))
        { }

        Pool<ObjectType>& operator=(Pool<ObjectType>&& other) noexcept {
            if (this != &other) {
                items = std::move(other.items);
                stack = std::move(other.stack);
            }
            return *this;
        }
        ObjectType& operator[](int index) {
            if (index < 0 || index >= static_cast<int>(items.size())) {
                throw std::out_of_range("Index out of range");
            }
            return items[index];
        }
    };









}