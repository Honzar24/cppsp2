#include <Mem_DB/CMemory_Database.hpp>

Result_type CMemory_Database::Find_Value(functor_type func) const
{
    Result_type result;
    for (const auto& [key, values] : base)
    {
        for (const auto& value : values)
        {
            if (func(key, value))
            {
                result.add(key, value);
            }
        }
    }
    return result;
}