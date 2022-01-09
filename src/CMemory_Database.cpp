#include <Mem_DB/CMemory_Database.hpp>

std::ostream& operator<<(std::ostream& s, const DB_variant_p& var)
{
    std::visit(visitors{
            [&](const int& i) {s << i;},
            [&](const double& d) {s << d;},
            [&](const std::string& str) {s << "\"" << str << "\"";}
        }, var);
    return s;
}

std::ostream& operator<<(std::ostream& s, const DB_variant& var)
{
    std::visit(visitors{
            [&](const int& i) {s << i;},
            [&](const double& d) {s << d;},
            [&](const std::string& str) {s << "\"" << str << "\"";},
            [&](const Pair& p) {s << p;}
        }, var);
    return s;
}

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