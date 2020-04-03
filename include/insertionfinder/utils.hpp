#pragma once
#include <iterator>
#include <type_traits>
#include <utility>

namespace InsertionFinder::Details {
    template <class T, class = void> struct is_iterable: std::false_type {};

    template <class T> struct is_iterable<T, std::void_t<
        decltype(std::begin(std::declval<T>())),
        decltype(std::end(std::declval<T>()))
    >>: std::true_type {};

    template <class T> constexpr bool is_iterable_v = is_iterable<T>::value;
};
