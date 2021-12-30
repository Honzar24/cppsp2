#pragma once

#include <concepts>
#include <map>
#include <vector>


#include <Mem_DB/p_types.hpp>


class CMemory_Database
{
public:

    enum class db_operation
    {
        KEY_EQUALS,
        KEY_GREATER,
        KEY_LESS
    };


    template<DB_type_primitive key_type, DB_type... V>
    Result_type Insert(key_type key, V... values);

    template<DB_type_primitive key_type, DB_type... V>
    Result_type Delete(key_type key, V... values);

    template<DB_type_primitive key_type>
    Result_type Search_Key(key_type key, db_operation op);


    // functor_type bude funktor/funkce/lambda, ktera splnuje koncept (vraci bool, zda vysledek vyhovuje nebo ne)
    Result_type Find_Value(functor_type func);

private:
    std::unordered_map<DB_variant,std::vector<DB_variant>> base;

};