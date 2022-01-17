#pragma once

#include <concepts>
#include <string>
#include <functional>
#include <vector>
#include <variant>
#include <compare>
#include <algorithm>
#include <iostream>

template<class... TS> struct visitors :TS... {using TS::operator()...;};

template<typename T>
concept DB_type_primitive = std::convertible_to<T, int> || std::convertible_to<T, double> || std::convertible_to<T, std::string>;

using DB_variant_p = std::variant<int, double, std::string>;

std::ostream& operator<<(std::ostream& s, const DB_variant_p& var) noexcept;


class Pair {
public:
    Pair() noexcept {};
    Pair(auto& f, auto& s) noexcept :first(f), second(s) {};
    Pair(auto&& f, auto&& s) noexcept :first(std::move(f)), second(std::move(s)) {};

    Pair(const Pair& c) noexcept :first(c.first), second(c.second) {};
    Pair(Pair&& m) noexcept :first(std::move(m.first)), second(std::move(m.second)) {};

    Pair& operator=(const Pair& c) noexcept
    {
        first = c.first;
        second = c.second;
        return *this;
    }

    Pair& operator=(Pair&& m) noexcept
    {
        first = std::move(m.first);
        second = std::move(m.second);
        return *this;
    }

    ~Pair() {};

    auto operator<=>(const Pair&) const = default;

    DB_variant_p getFirst() const noexcept
    {
        return first;
    }
    DB_variant_p getSecond() const noexcept
    {
        return second;
    }

    friend std::ostream& operator<<(std::ostream& s, const Pair& p) noexcept
    {
        s << "[";
        s << p.first;
        s << ":";
        s << p.second;
        s << "]";
        return s;
    }

private:
    DB_variant_p first;
    DB_variant_p second;
};


template<typename T>
concept DB_type = DB_type_primitive<T> || std::derived_from<T, Pair>;


using DB_variant = std::variant<int, double, std::string, Pair>;

std::ostream& operator<<(std::ostream& s, const DB_variant& var) noexcept;


using functor_type = std::function<bool(const DB_variant& key, const DB_variant& value)>;

