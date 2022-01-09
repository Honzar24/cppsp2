#pragma once

#include <cassert>

#include <concepts>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <functional>
#include <iostream>

#include <Mem_DB/p_types.hpp>


template<typename T>
concept text = std::convertible_to<std::string, T> || std::same_as<const char*, T>;
class CMemory_Database
{
private:
    using base_type = std::map<DB_variant, std::vector<DB_variant>>;
    base_type base;

public:

    enum class operation
    {
        KEY_EQUALS,
        KEY_GREATER,
        KEY_LESS
    };

    constexpr bool compare(auto&& order, const operation& op) const noexcept
    {
        using enum operation;
        switch (op)
        {
        case KEY_EQUALS:    return (order == std::strong_ordering::equal) || (order == std::strong_ordering::equivalent);
        case KEY_GREATER:   return order == std::strong_ordering::greater;
        case KEY_LESS:      return order == std::strong_ordering::less;

        default:
            assert(false && "unreachable");
            return false;
        }
    }


    CMemory_Database() noexcept : base(base_type()) {};

    template<DB_type key_type, DB_type... Vals>
    Result_type Insert(key_type key, Vals... values) noexcept
    {
        Result_type result;
        auto& vect = base[key];
        m_insert(vect, values...);
        result.add(key, vect);
        return result;
    }

    template<DB_type key_type>
    Result_type Delete(key_type key) noexcept
    {
        Result_type result;
        //auto& vect = base[key];
        base[key] = {};
        result.add(key, base[key]);
        return result;
    }

    template<DB_type key_type, DB_type... V>
    Result_type Delete(key_type key, V... values) noexcept
    {
        Result_type result;
        auto& vect = base[key];
        m_delete(vect, values...);
        result.add(key, vect);
        return result;
    }

    template<DB_type key_type>
    Result_type Search_Key(const key_type& key, const operation& op) const noexcept
    {
        Result_type result;
        for (const auto& [key_v, values] : base)
        {
            if constexpr (std::is_arithmetic_v<key_type>)
            {
                std::visit(visitors{
                        [&](const int& i) {
                            if (compare(i <=> key,op))
                            {
                                result.add(i,values);
                            }
                         },
                        [&](const double& d) {
                            if (compare(d <=> key,op))
                            {
                                result.add(d,values);
                            }
                        },
                        [&](const auto&) {/*not comparable*/},
                        [&](const Pair&) {/*not comparable*/}
                    }, key_v);
            } else if constexpr (std::is_convertible_v<key_type, std::string>)
            {
                std::visit(visitors{
                        [&](const int&) {/*not comparable*/},
                        [&](const double&) {/*not comparable*/},
                        [&](const std::string& s) {
                            if (compare(key <=> s,op))
                            {
                                result.add(s,values);
                            }
                         },
                         [&](const Pair&) {/*not comparable*/}
                    }, key_v);
            } else if constexpr (std::derived_from<key_type, Pair>)
            {
                std::visit(visitors{
                        [&](const int&) {/*not comparable*/},
                        [&](const double&) {/*not comparable*/},
                        [&](const std::string&) {/*not comparable*/},
                        [&](const Pair& p) {
                            if (compare(key <=> p,op))
                            {
                                result.add(p,values);
                            }
                         }
                    }, key_v);
            } else
            {
                assert(false && "unreachable");
            }
        }
        return result;
    }



    // functor_type bude funktor/funkce/lambda, ktera splnuje koncept (vraci bool, zda vysledek vyhovuje nebo ne)
    Result_type Find_Value(functor_type func) const noexcept;

private:
    template<DB_type V>
    void m_insert(std::vector<DB_variant>& vect, V value) noexcept
    {
        vect.push_back(value);
    }

    template<DB_type V, DB_type... VALS>
    void m_insert(std::vector<DB_variant>& vect, V value, VALS... values) noexcept
    {
        vect.push_back(value);
        m_insert(vect, values...);
    }

    template<DB_type V>
    void m_delete(std::vector<DB_variant>& vect, V value) noexcept
    {
        DB_variant val(value);
        vect.erase(std::remove(vect.begin(), vect.end(), val));
    }

    template<DB_type V, DB_type... VALS>
    void m_delete(std::vector<DB_variant>& vect, V value, VALS... values) noexcept
    {
        DB_variant val(value);
        vect.erase(std::remove(vect.begin(), vect.end(), val));
        Delete(vect, values...);
    }
};
