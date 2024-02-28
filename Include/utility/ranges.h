#pragma once
#include "Span.h"
#include "Template.h"
namespace utility
{
	struct search
	{
        template<typename Function, typename IT>
            requires Template::lambda_signature<Function, bool, std::remove_cvref_t<decltype(*std::declval<IT>())>>
        static IT find_if(IT left, IT right, Function func)
        {
            while (left != right)
            {
                if (func(*left))
                {
                    break;
                }
                ++left;
            }
            return left;
        }




	};
}