#pragma once
#include <tuple>
#include <array>
#include <cstddef>
#include <utility>
namespace utility
{


    template <typename Tuple, std::size_t... I>
    void assignTupleFromArray(Tuple& tuple, const std::array<void*, sizeof...(I)>& array, std::index_sequence<I...>) {
        ((std::get<I>(tuple) = reinterpret_cast<typename std::tuple_element<I, Tuple>::tokenType>(array[I])), ...);
    }

}
namespace Template {


    template<typename, typename T>
    struct is_member_function_pointer_of_type : std::false_type {};

    template<typename R, typename C>
    struct is_member_function_pointer_of_type<R(C::*)(), C> : std::true_type {};
    template<typename Func, typename Obj>
    inline constexpr bool is_member_function_pointer_of_type_v = is_member_function_pointer_of_type<Func, Obj>::value;

    template<typename Func, typename Obj, typename... Args>
    concept MemberFunctionPointer = requires(Func f, Obj * obj, Args... args) {
        { (obj->*f)(args...) };
    };

    template<typename T, typename = void>
    struct FirstElementOrDefaultType {
        using type = int;
    };

    template<typename T>
    struct FirstElementOrDefaultType<T, std::enable_if_t<(std::tuple_size<T>::value > 0)>> {
        using type = decltype(std::get<0>(std::declval<T>()));
    };

    template<typename T>
    using FirstElementOrDefaultType_t = typename FirstElementOrDefaultType<T>::type;


    template<typename T>
    struct function_traits;

    // For const member functions (already in your code)
    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...) const> {
        using return_type = R;
        using args = std::tuple<Args...>;
    };

    // For non-const member functions (handles mutable lambdas)
    template<typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...)> {
        using return_type = R;
        using args = std::tuple<Args...>;
    };

    template<typename Lambda>
    using lambda_traits = function_traits<decltype(&Lambda::operator())>;

    template<typename T>
    concept Callable = requires(T t) {
        { &T::operator() };
    };

    template<typename T, typename... Rest>
    struct FirstTypeSelector {
        using type = T;
    };

    template<>
    struct FirstTypeSelector<std::tuple<>> {
        using type = int;
    };
    template<typename Func>
        requires Callable<Func>
    struct Lambda {
    private:
        using Traits = lambda_traits<Func>;

    public:
        using retType = typename Traits::return_type;
        using argsTuple = typename Traits::args;

        static constexpr size_t args_count = std::tuple_size_v<argsTuple>;

        using firstParameter = std::remove_reference_t<FirstElementOrDefaultType_t<argsTuple>>;
        static constexpr bool is_void()
        {
            return std::is_same_v<retType, void>;
        }

       
    };
    template<typename Tuple1, typename Tuple2, std::size_t... I>
    constexpr bool are_tuples_constructible(std::index_sequence<I...>) {
        return (std::is_constructible_v<std::tuple_element_t<I, Tuple1>, std::tuple_element_t<I, Tuple2>> && ...);
    }


    template<typename Function, typename Return, typename... Args>
    concept lambda_signature = requires {
        typename Lambda<Function>::retType;
        typename Lambda<Function>::argsTuple;

            requires std::is_same_v<typename Lambda<Function>::retType, Return>;

            requires are_tuples_constructible<typename Lambda<Function>::argsTuple, std::tuple<Args...>>(
        std::make_index_sequence<std::tuple_size_v<typename Lambda<Function>::argsTuple>>{});
    };

   



}