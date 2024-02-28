#pragma once
#pragma once

#include <vector>
#include <string>
#include "Span.h"
#include "SequenceArray.h"

namespace re
{


    struct RawFinder
    {
    private:
        const char* start_inclusive;
        const char* end_exclusive;
        char* current_start;

    public:
        RawFinder(const char* start_inclusive, const char* end_exclusive);
        RawFinder();
        RawFinder(StringView view);

        template<typename Function, typename... Args>
        std::vector<StringView> capture_all(Function func, Args&&... args)
        {
            std::vector<StringView> result;
            StringView view = std::invoke(func, *this, std::forward<Args>(args)...);
            result.emplace_back(view);

            while (current_start < end_exclusive)
            {
                view = std::invoke(func, *this, std::forward<Args>(args)...);
                result.back() += view;
                if (!view.empty())
                {
                    result.emplace_back(view);
                }
            }
            Reset();

            return result;
        }


        StringView capture_next(SequenceArray& patterns);
        StringView capture_next_nchar(const std::string& characters);
        StringView capture_next_char(const char character);

        template<bool include_enter, bool include_exit>
        StringView capture_from_to(SequenceArray& from, SequenceArray& to)
        {
            char* enter;
            char* exit;
            if (include_enter)
                enter = capture_next(from).data_ptr;
            else
                enter = capture_next(from).end_ptr;
            if (include_exit)
                exit = capture_next(to).end_ptr;
            else
                exit = capture_next(to).data_ptr;

            return { enter, exit };
        }

        template<bool include_enter, bool include_exit>
        StringView capture_from_to_nchar(const std::string& from, const std::string& to)
        {
            char* enter;
            char* exit;
            if (include_enter)
                enter = capture_next_nchar(from).data_ptr;
            else
                enter = capture_next_nchar(from).end_ptr;
            if (include_exit)
                exit = capture_next_nchar(to).end_ptr;
            else
                exit = capture_next_nchar(to).data_ptr;

            return { enter, exit };
        }

        template<bool include_enter, bool include_exit>
        StringView capture_from_to_char(const char from, const char to)
        {
            char* enter;
            char* exit;
            if (include_enter)
                enter = capture_next_char(from).data_ptr;
            else
                enter = capture_next_char(from).end_ptr;
            if (include_exit)
                exit = capture_next_char(to).end_ptr;
            else
                exit = capture_next_char(to).data_ptr;

            return { enter, exit };
        }

        std::vector<char> replace_all(std::vector<std::string> replaces, std::vector<IndexedStringView> places);




        bool done();
        void Reset();
        void operator++();
        char& current();
        void jump_to(char* it);

    };











}

using StringViewIterator = re::RawFinder;
