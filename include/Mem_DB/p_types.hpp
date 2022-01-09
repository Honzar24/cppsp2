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

std::ostream& operator<<(std::ostream& s, const DB_variant_p& var);


class Pair {
public:
    Pair() {};
    Pair(auto& f, auto& s) :first(f), second(s) {};
    Pair(auto&& f, auto&& s) :first(std::move(f)), second(std::move(s)) {};

    Pair(const Pair& c) :first(c.first), second(c.second) {};
    Pair(Pair&& m) :first(std::move(m.first)), second(std::move(m.second)) {};

    Pair& operator=(const Pair& c)
    {
        first = c.first;
        second = c.second;
        return *this;
    }

    Pair& operator=(const Pair&& m)
    {
        first = std::move(m.first);
        second = std::move(m.second);
        return *this;
    }

    ~Pair() {};

    auto operator<=>(const Pair&) const = default;

    DB_variant_p getFirst()const
    {
        return first;
    }
    DB_variant_p getSecond()const
    {
        return second;
    }

    friend std::ostream& operator<<(std::ostream& s, const Pair& p)
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

std::ostream& operator<<(std::ostream& s, const DB_variant& var);


class Record {
public:
    Record(const DB_variant& key, const std::vector<DB_variant>& vals) :key(key), values(vals) {};
    Record(const DB_variant& key, const DB_variant& value) :key(key), values({})
    {
        values.push_back(value);
    };

    void add(const DB_variant& value)
    {
        values.push_back(value);
    }
    void add(const std::vector<DB_variant> values)
    {
        std::copy(values.begin(), values.end(), this->values.begin());
    }

    std::vector<DB_variant> getValues() const
    {
        return values;
    }

    friend std::ostream& operator<<(std::ostream& s, const Record& r)
    {
        s << r.key;
        s << " - ";
        size_t i;
        for (i = 0; i < (r.values.size() - 1); i++)
        {
            s << r.values[i] << ", ";
        }
        if (r.values.size() != 0)
        {
            s << r.values[i];
        }
        return s;
    }

public:
    const DB_variant key;
private:
    std::vector<DB_variant> values;
};


using functor_type = std::function<bool(const DB_variant& key, const DB_variant& value)>;

class Result_type
{
public:
    Result_type() :status(true), lines(0), records({}) {};
    Result_type(bool status, size_t lines) :status(status), lines(lines), records({}) {};
    Result_type(size_t lines) :Result_type(true, lines) {};
    void add(const Record& rec)
    {
        records.push_back(rec);
        lines++;
    }
    void add(const DB_variant& key, const DB_variant& value)
    {
        auto ri = std::find_if(records.begin(), records.end(), [&key](const Record& i) {return key == i.key;});
        if (ri != records.end())
        {
            (*ri).add(value);
            return;
        }
        records.emplace_back(Record(key, value));
        lines++;
    }
    void add(const DB_variant& key, const std::vector<DB_variant>& values)
    {
        auto ri = std::find_if(records.begin(), records.end(), [&key](const Record& i) {return key == i.key;});
        if (ri != records.end())
        {
            (*ri).add(values);
            return;
        }
        records.emplace_back(Record(key, values));
        lines++;
    }

    inline size_t getLines()const
    {
        return lines;
    }

    inline bool empty()const
    {
        return records.size() == 0;
    }

    std::vector<DB_variant> getValues() const
    {
        std::vector<DB_variant> ret;
        for (auto& record : records)
        {
            const auto& vect = record.getValues();
            ret.insert(ret.end(), vect.begin(), vect.end());
        }
        return ret;
    }

    friend std::ostream& operator<<(std::ostream& s, const Result_type& r)
    {
        std::string t = r.status ? "OK" : "ERROR";
        s << t << std::endl;
        s << r.lines << " rows." << std::endl;
        if (r.records.size() == 0)
        {
            s << "Empty." << std::endl;
            return s;
        }
        for (const auto& record : r.records)
        {
            s << record << std::endl;
        }
        return s;
    }
private:
    bool status;
    size_t lines;
    std::vector<Record> records;
};


