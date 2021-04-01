#pragma once

#include <iterator>
#include <concepts>

namespace util
{
    template <std::integral T>
    class range_iterator final
    {
        T idx;

    public:
        using difference_type = std::make_signed_t<T>;
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

        constexpr range_iterator(T v = 0) noexcept : idx(v) {}

        // Increment and decrement
        constexpr range_iterator& operator++() noexcept { ++idx; return *this; }
        constexpr range_iterator operator++(int) noexcept { return range_iterator(idx++); }
        constexpr range_iterator& operator--() noexcept { --idx; return *this; }
        constexpr range_iterator operator--(int) noexcept { return range_iterator(idx--); }

        // Dereference
        constexpr const T& operator*() const noexcept { return idx; }

        // Addition, subtraction, array index
        constexpr range_iterator& operator+=(difference_type n) noexcept { idx += n; return *this; }
        constexpr range_iterator& operator-=(difference_type n) noexcept { idx -= n; return *this; }
        
        friend constexpr range_iterator operator+(const range_iterator& it, difference_type n) noexcept
        {
            return range_iterator(it.idx + n);
        }

        friend constexpr range_iterator operator+(difference_type n, const range_iterator& it) noexcept
        {
            return range_iterator(n + it.idx);
        }

        friend constexpr range_iterator operator-(const range_iterator& it, difference_type n) noexcept
        {
            return range_iterator(it.idx - n);
        }

        friend constexpr difference_type operator-(const range_iterator& it1, const range_iterator& it2) noexcept
        {
            return it1.idx - it2.idx;
        }

        constexpr T operator[](difference_type n) const noexcept { return idx + n; }

        // Comparison
        friend constexpr bool operator==(const range_iterator& it1, const range_iterator& it2) noexcept { return it1.idx == it2.idx; }
        friend constexpr bool operator!=(const range_iterator& it1, const range_iterator& it2) noexcept { return it1.idx != it2.idx; }
        friend constexpr bool operator<(const range_iterator& it1, const range_iterator& it2) noexcept { return it1.idx < it2.idx; }
        friend constexpr bool operator>(const range_iterator& it1, const range_iterator& it2) noexcept { return it1.idx > it2.idx; }
        friend constexpr bool operator<=(const range_iterator& it1, const range_iterator& it2) noexcept { return it1.idx <= it2.idx; }
        friend constexpr bool operator>=(const range_iterator& it1, const range_iterator& it2) noexcept { return it1.idx >= it2.idx; }
    };

    template <std::integral T>
    class range final
    {
        T _begin, _end;

    public:
        constexpr range(T begin, T end) noexcept : _begin(begin), _end(end) {}
        constexpr range(T end) noexcept : _begin(), _end(end) {}

        constexpr auto begin() const noexcept { return range_iterator(_begin); }
        constexpr auto cbegin() const noexcept { return range_iterator(_begin); }

        constexpr auto end() const noexcept { return range_iterator(_end); }
        constexpr auto cend() const noexcept { return range_iterator(_end); }

        constexpr auto rbegin() const noexcept { return std::make_reverse_iterator(range_iterator(_end - 1)); }
        constexpr auto crbegin() const noexcept { return std::make_reverse_iterator(range_iterator(_end - 1)); }

        constexpr auto rend() const noexcept { return std::make_reverse_iterator(range_iterator(_begin - 1)); }
        constexpr auto crend() const noexcept { return std::make_reverse_iterator(range_iterator(_begin - 1)); }
    };

    template <std::integral T>
    range(T begin, T end)->range<T>;

    template <std::integral T, std::integral U>
    range(T begin, U end)->range<std::common_type_t<T, U>>;
}
