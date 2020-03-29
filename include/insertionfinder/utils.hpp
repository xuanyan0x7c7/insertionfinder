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

namespace InsertionFinder::Details {
    template<class K, class V> struct Mapping: std::pair<K, V> {
        template <class T> Mapping(T&& arg): std::pair<K, V>(std::forward<T>(arg), V()) {}
        template <class T, class U> Mapping(const std::pair<T, U>& arg): std::pair<K, V>(arg) {}
    };
};
