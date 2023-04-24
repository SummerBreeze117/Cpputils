//
// Created by co2ma on 2023/4/14.
//

#ifndef CODE_CURRY_H
#define CODE_CURRY_H

#include <type_traits>
#include <tuple>
#include <functional>

// ---

template <typename FType>
struct function_traits;

template <typename RType, typename... ArgTypes>
struct function_traits<RType(ArgTypes...)> {
    using arity = std::integral_constant<size_t, sizeof...(ArgTypes)>;

    using result_type = RType;

    template <size_t Index>
    using arg_type = typename std::tuple_element<Index, std::tuple<ArgTypes...>>::type;
};

// ---

namespace details {
    template <typename T>
    struct function_type_impl
            : function_type_impl<decltype(&T::operator())>
    { };

    template <typename RType, typename... ArgTypes>
    struct function_type_impl<RType(ArgTypes...)> {
        using type = RType(ArgTypes...);
    };

    template <typename RType, typename... ArgTypes>
    struct function_type_impl<RType(*)(ArgTypes...)> {
        using type = RType(ArgTypes...);
    };

    template <typename RType, typename... ArgTypes>
    struct function_type_impl<std::function<RType(ArgTypes...)>> {
        using type = RType(ArgTypes...);
    };

    template <typename T, typename RType, typename... ArgTypes>
    struct function_type_impl<RType(T::*)(ArgTypes...)> {
        using type = RType(ArgTypes...);
    };

    template <typename T, typename RType, typename... ArgTypes>
    struct function_type_impl<RType(T::*)(ArgTypes...) const> {
        using type = RType(ArgTypes...);
    };
}

template <typename T>
struct function_type
        : details::function_type_impl<typename std::remove_cv<typename std::remove_reference<T>::type>::type>
{ };

// ---

template <typename Args, typename Params>
struct apply_args;

template <typename HeadArgs, typename... Args, typename HeadParams, typename... Params>
struct apply_args<std::tuple<HeadArgs, Args...>, std::tuple<HeadParams, Params...>>
        : std::enable_if<
                std::is_constructible<HeadParams, HeadArgs>::value,
                apply_args<std::tuple<Args...>, std::tuple<Params...>>
        >::type
{ };

template <typename... Params>
struct apply_args<std::tuple<>, std::tuple<Params...>> {
    using type = std::tuple<Params...>;
};

// ---

template <typename TupleType>
struct is_empty_tuple : std::false_type { };

template <>
struct is_empty_tuple<std::tuple<>> : std::true_type { };

// ----

template <typename FType, typename GivenArgs, typename RestArgs>
struct currying;

template <typename FType, typename... GivenArgs, typename... RestArgs>
struct currying<FType, std::tuple<GivenArgs...>, std::tuple<RestArgs...>> {
    std::tuple<GivenArgs...> given_args;

    FType func;

    template <typename Func, typename... GivenArgsReal>
    constexpr
    currying(Func&& func, GivenArgsReal&&... args) :
            given_args(std::forward<GivenArgsReal>(args)...),
            func(std::move(func))
    { }

    template <typename... Args>
    constexpr
    auto operator() (Args&&... args) const& {
        using ParamsTuple = std::tuple<RestArgs...>;
        using ArgsTuple = std::tuple<Args...>;

        using RestArgsPrime = typename apply_args<ArgsTuple, ParamsTuple>::type;

        using CanExecute = is_empty_tuple<RestArgsPrime>;

        return apply(CanExecute{}, std::make_index_sequence<sizeof...(GivenArgs)>{}, std::forward<Args>(args)...);
    }

    template <typename... Args>
    constexpr
    auto operator() (Args&&... args) && {
        using ParamsTuple = std::tuple<RestArgs...>;
        using ArgsTuple = std::tuple<Args...>;

        using RestArgsPrime = typename apply_args<ArgsTuple, ParamsTuple>::type;

        using CanExecute = is_empty_tuple<RestArgsPrime>;

        return std::move(*this).apply(CanExecute{}, std::make_index_sequence<sizeof...(GivenArgs)>{}, std::forward<Args>(args)...);
    }

private:
    template <typename... Args, size_t... Indices>
    constexpr
    auto apply(std::false_type, std::index_sequence<Indices...>, Args&&... args) const& {
        using ParamsTuple = std::tuple<RestArgs...>;
        using ArgsTuple = std::tuple<Args...>;

        using RestArgsPrime = typename apply_args<ArgsTuple, ParamsTuple>::type;

        using CurryType = currying<FType, std::tuple<GivenArgs..., Args...>, RestArgsPrime>;

        return CurryType{ func, std::get<Indices>(given_args)..., std::forward<Args>(args)... };
    }

    template <typename... Args, size_t... Indices>
    constexpr
    auto apply(std::false_type, std::index_sequence<Indices...>, Args&&... args) && {
        using ParamsTuple = std::tuple<RestArgs...>;
        using ArgsTuple = std::tuple<Args...>;

        using RestArgsPrime = typename apply_args<ArgsTuple, ParamsTuple>::type;

        using CurryType = currying<FType, std::tuple<GivenArgs..., Args...>, RestArgsPrime>;

        return CurryType{ std::move(func), std::get<Indices>(std::move(given_args))..., std::forward<Args>(args)... };
    }

    template <typename... Args, size_t... Indices>
    constexpr
    auto apply(std::true_type, std::index_sequence<Indices...>, Args&&... args) const& {
        return func(std::get<Indices>(given_args)..., std::forward<Args>(args)...);
    }

    template <typename... Args, size_t... Indices>
    constexpr
    auto apply(std::true_type, std::index_sequence<Indices...>, Args&&... args) && {
        return func(std::get<Indices>(std::move(given_args))..., std::forward<Args>(args)...);
    }
};

// ---

template <typename FType, size_t... Indices>
constexpr
auto curry(FType&& func, std::index_sequence<Indices...>) {
    using RealFType = typename function_type<FType>::type;
    using FTypeTraits = function_traits<RealFType>;

    using CurryType = currying<FType, std::tuple<>, std::tuple<typename FTypeTraits::template arg_type<Indices>...>>;

    return CurryType{ std::move(func) };
}

template <typename FType>
constexpr
auto curry(FType&& func) {
    using RealFType = typename function_type<FType>::type;
    using FTypeArity = typename function_traits<RealFType>::arity;

    return curry(std::move(func), std::make_index_sequence<FTypeArity::value>{});
}

#endif //CODE_CURRY_H
