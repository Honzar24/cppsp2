#pragma once

#include <concepts>
#include <string>
#include <functional>
#include <vector>
#include <variant>


template<typename T>
concept DB_type_primitive = std::convertible_to<T,int> || std::convertible_to<T,double> || std::convertible_to<T,std::string>;

using DB_variant_p = std::variant<int,double,std::string>;


class Pair{
    public:
    Pair();

    private:
    DB_variant_p first;
    DB_variant_p second;
};

template<typename T>
concept DB_type = DB_type_primitive<T> || std::same_as<Pair, T>;

using DB_variant = std::variant<int,double,std::string,Pair>;


class Record{
    public:


    public:
    const DB_variant key;
    private:
    std::vector<DB_variant> values;
};


//tato kombinace(template/using) funguje to jsem necekal
using functor_type = std::function<bool(Record)>;

class Result_type
{
private:
    bool status;
    size_t lines;
    std::vector<Record> records;
};
