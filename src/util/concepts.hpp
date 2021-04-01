#pragma once

#include <concepts>
#include <type_traits>

namespace util
{
    template <typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;

    template <typename T, typename U>
    concept explicitly_convertible_to = std::constructible_from<U, T> && !std::is_convertible_v<T, U>;
}
