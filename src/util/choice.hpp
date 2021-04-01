#pragma once

namespace util
{
    template <typename Key, auto Value> struct choice;

    template <typename Key, typename... Choices> struct choice_list {};

    template <typename Key, auto Value, typename... NextChoices>
    struct choice_list<Key, choice<Key, Value>, NextChoices...>
    {
        static constexpr auto value = Value;
    };

    template <typename Key, typename Choice, typename... NextChoices>
    struct choice_list<Key, Choice, NextChoices...> : choice_list<Key, NextChoices...> {};

    template <typename Key, typename... Choices>
    constexpr auto choice_list_v = choice_list<Key, Choices...>::value;
}
