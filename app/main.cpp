#include <cmath>

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <concepts>
#include <limits>
#include <filesystem>
#include <fstream>
#include <exception>

#include <Mem_DB/CMemory_Database.hpp>

using fspath = std::filesystem::path;


std::regex command_regex("([A-Z_]+)\\((.*)\\)", std::regex::ECMAScript);
constexpr size_t command_regex_command = 1;
constexpr size_t command_regex_args = 2;

std::regex args_regex("[^,\\s][^\\,\\n]*[^,\\s]*", std::regex::ECMAScript);

std::regex int_regex("^[-+]?\\d*$", std::regex::ECMAScript);
std::regex double_regex("\\d?\\.\\d?", std::regex::ECMAScript);
std::regex string_regex("\"([^\"]*)\"", std::regex::ECMAScript);
//                         key  value
std::regex pair_regex("\\[(.+):(.+)\\]", std::regex::ECMAScript);
constexpr size_t pair_regex_key = 1;
constexpr size_t pair_regex_value = 2;

DB_variant_p parse_arg_p(std::string&& arg)
{
    std::smatch match;
    if (std::regex_search(arg, match, string_regex))
    {
        return DB_variant_p(match[1].str());
    } else if (std::regex_search(arg, match, double_regex))
    {
        return DB_variant_p(std::stod(match[0].str()));
    } else if (std::regex_search(arg, match, int_regex))
    {
        return DB_variant_p(std::stoi(match[0].str()));
    } else
    {
        std::runtime_error ex(arg + "  is not valid [int|double|string]!");
        throw ex;
    }
}

DB_variant parse_arg(std::string&& arg)
{
    std::smatch match;
    if (std::regex_search(arg, match, pair_regex))
    {
        Pair ret;
        auto key = match[pair_regex_key].str();
        auto value = match[pair_regex_value].str();
        if (std::regex_match(key, pair_regex) || std::regex_match(value, pair_regex))
        {
            std::runtime_error ex("Pair in pair is not valid!");
            throw ex;
        }
        std::visit([&](auto&& first, auto&& second) {ret = Pair(first, second);},
            parse_arg_p(std::move(key)),
            parse_arg_p(std::move(value))
        );
        return ret;
    } else
    {
        DB_variant ret;
        std::visit([&](auto&& val) {ret = DB_variant(val);}, parse_arg_p(std::move(arg)));
        return ret;
    }
}


std::vector<DB_variant> parse_Args(std::string&& args)
{
    std::vector<DB_variant> vars;
    for (auto itr = std::sregex_iterator(args.begin(), args.end(), args_regex); itr != std::sregex_iterator(); ++itr)
    {
        auto arg = itr->str();
        vars.push_back(std::move(parse_arg(std::move(arg))));
    }
    return vars;
}

enum class Directiv : size_t {
    UNKNOWN,
    EXIT,
    INSERT,
    DELETE,
    KEY_EQUALS,
    KEY_GREATER,
    KEY_LESS,
    FIND_VALUE,
    AVERAGE,
    MIN,
    MAX,
};

using enum Directiv;
constexpr auto COMMANDS = { INSERT,DELETE,KEY_EQUALS,KEY_GREATER,KEY_LESS,FIND_VALUE,AVERAGE,MIN,MAX,EXIT };

constexpr  std::string_view directiv_to_str(const Directiv& com) noexcept
{
    switch (com)
    {
    case INSERT:      return "INSERT";
    case DELETE:      return "DELETE";
    case KEY_EQUALS:  return "KEY_EQUALS";
    case KEY_GREATER: return "KEY_GREATER";
    case KEY_LESS:    return "KEY_LESS";
    case FIND_VALUE:  return "FIND_VALUE";
    case AVERAGE:     return "AVERAGE";
    case MIN:         return "MIN";
    case MAX:         return "MAX";
    case EXIT:        return "EXIT";
    default:          return "UNKNOWN";
    }
}

std::ostream& operator<<(std::ostream& s, const Directiv com) noexcept
{
    s << directiv_to_str(com);
    return s;
}
class Command {
public:
    Command(Directiv d, std::string&& args) noexcept :directiv(d), args(std::move(args)) {}

    Command(const Command& o) noexcept
    {
        directiv = o.directiv;
        args = o.args;
    };
    Command(Command&& o) noexcept
    {
        directiv = o.directiv; o.directiv = UNKNOWN;
        args = std::move(o.args);
    };


    Command& operator=(const Command& o) noexcept
    {
        directiv = o.directiv;
        args = o.args;
        return *this;
    }

    Command& operator=(Command&& o) noexcept
    {
        directiv = o.directiv; o.directiv = UNKNOWN;
        args = std::move(o.args);
        return *this;
    }

    ~Command() {};

    friend std::ostream& operator<<(std::ostream& s, Command& c) noexcept
    {
        s << c.directiv << "(" << c.args << ")";
        return s;
    };

    inline Directiv getDirectiv() const noexcept
    {
        return directiv;
    }

    inline std::string getArgs() const noexcept
    {
        return args;
    }

private:
    Directiv directiv;
    std::string args;
};



Command parseCommand(std::istream& stream)
{
    std::string line;
    stream >> line;
    std::smatch match;
    if (!std::regex_search(line, match, command_regex))
    {
        if (line.compare(directiv_to_str(EXIT)) == 0)
        {
            return { EXIT,"" };
        }
        return { UNKNOWN,"" };
    }
    auto command = match[command_regex_command].str();
    for (auto& com : COMMANDS)
    {
        if (command.compare(directiv_to_str(com)) == 0)
        {
            return { com,match[command_regex_args].str() };
        }
    }
    return { UNKNOWN,"" };
}

Command loadCommand()
{
    std::cout << ">";
    Command com = parseCommand(std::cin);
    while (com.getDirectiv() == UNKNOWN)
    {
        std::cout << "Unknow command try again!" << std::endl;
        std::cout << ">";
        com = parseCommand(std::cin);
    }
    return com;
}

void execute_command(std::ostream& out, CMemory_Database& db, const Command& com)
{
    auto args = parse_Args(std::move(com.getArgs()));
    size_t i = 1;
    std::string msg;
    switch (com.getDirectiv())
    {
    case INSERT:
        if (args.size() < 2)
        {
            throw std::runtime_error("Minimal number of arguments for INSERT is 2!");
        }
        for (;i < args.size() - 1;i++)
        {
            std::visit([&](auto&& key, auto&& val) {db.Insert(key, val);}, args[0], args[i]);
        }
        std::visit([&](auto&& key, auto&& val) {out << db.Insert(key, val);}, args[0], args[i]);
        return;
    case DELETE:
        if (args.size() == 1)
        {
            std::visit([&](auto&& key) {out << db.Delete(key);}, args[0]);
            return;
        }
        for (;i < args.size() - 1;i++)
        {
            std::visit([&](auto&& key, auto&& val) {db.Delete(key, val);}, args[0], args[i]);
        }
        std::visit([&](auto&& key, auto&& val) {out << db.Delete(key, val);}, args[0], args[i]);
        return;

    case KEY_EQUALS:
        if (args.size() == 1)
        {
            std::visit([&](auto&& key) {out << db.Search_Key(key, CMemory_Database::operation::KEY_EQUALS);}, args[0]);
            return;
        }
        [[fallthrough]];
    case KEY_LESS:
        if (args.size() == 1)
        {
            std::visit([&](auto&& key) {out << db.Search_Key(key, CMemory_Database::operation::KEY_LESS);}, args[0]);
            return;
        }
        [[fallthrough]];
    case KEY_GREATER:
        if (args.size() == 1)
        {
            std::visit([&](auto&& key) {out << db.Search_Key(key, CMemory_Database::operation::KEY_GREATER);}, args[0]);
            return;
        }
        msg += "Wrong number of arguments for ";
        msg += directiv_to_str(com.getDirectiv());
        msg += " expected 1 got ";
        msg += std::to_string(args.size());
        throw std::runtime_error(msg);
    case AVERAGE:
    {
        double sum = 0;
        size_t count = 0;
        if (args.size() == 0)
        {
            auto result = db.Find_Value([&](const DB_variant&, const DB_variant& value) {
                bool ret = false;
                std::visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_arithmetic_v<T>)
                    {
                        sum += val;
                        count++;
                        ret = true;
                    }
                    }, value);
                return ret;});
            if (count == 0)
            {
                out << "ERROR" << std::endl;
                out << "No values is valid type" << std::endl;
                return;
            }
            out << "OK" << std::endl;
            out << result.getLines() << " rows." << std::endl;
            out << "AVERAGE - " << sum / count << std::endl;
            return;
        }
        if (args.size() == 1)
        {
            std::visit([&](auto&& key) {
                const auto result = db.Search_Key(key, CMemory_Database::operation::KEY_EQUALS);
                const auto values = result.getValues();
                for (const auto& value : values)
                {
                    std::visit([&](auto&& val) {
                        using T = std::decay_t<decltype(val)>;
                        if constexpr (std::is_arithmetic_v<T>)
                        {
                            sum += val;
                            count++;
                        }
                        }, value);
                }
                if (count == 0)
                {
                    out << "ERROR" << std::endl;
                    out << "No values is valid type" << std::endl;
                    return;
                }
                out << "OK" << std::endl;
                out << result.getLines() << " rows." << std::endl;
                out << "AVERAGE - " << sum / count << std::endl;
                }, args[0]);
            return;
        }
    }
    [[fallthrough]];
    case MIN:
    {
        double min = std::numeric_limits<double>::max();
        if (args.size() == 0)
        {
            auto result = db.Find_Value([&](const DB_variant&, const DB_variant& value) {
                bool ret = false;
                std::visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_arithmetic_v<T>)
                    {
                        min = std::min(min, static_cast<double>(val));
                        ret = true;
                    }
                    }, value);
                return ret;});
            out << "OK" << std::endl;
            out << result.getLines() << " rows." << std::endl;
            out << "MIN - " << min << std::endl;
            return;
        }
        if (args.size() == 1)
        {
            std::visit([&](auto&& key) {
                const auto result = db.Search_Key(key, CMemory_Database::operation::KEY_EQUALS);
                const auto values = result.getValues();
                if (values.size() == 0)
                {
                    out << "ERROR" << std::endl;
                    out << "No values is valid type" << std::endl;
                    return;
                }
                for (const auto& value : values)
                {
                    std::visit([&](auto&& val) {
                        using T = std::decay_t<decltype(val)>;
                        if constexpr (std::is_arithmetic_v<T>)
                        {
                            min = std::min(min, static_cast<double>(val));
                        }
                        }, value);
                }
                out << "OK" << std::endl;
                out << result.getLines() << " rows." << std::endl;
                out << "MIN - " << min << std::endl;
                }, args[0]);
            return;
        }
    }
    [[fallthrough]];
    case MAX:
    {
        double max = std::numeric_limits<double>::min();
        if (args.size() == 0)
        {
            auto result = db.Find_Value([&](const DB_variant&, const DB_variant& value) {
                bool ret = false;
                std::visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_arithmetic_v<T>)
                    {
                        max = std::max(max, static_cast<double>(val));
                        ret = true;
                    }
                    }, value);
                return ret;});
            if (result.empty())
            {
                out << "ERROR" << std::endl;
                out << "No values is valid type" << std::endl;
                return;
            }
            out << "OK" << std::endl;
            out << result.getLines() << " rows." << std::endl;
            out << "MAX - " << max << std::endl;
            return;
        }
        if (args.size() == 1)
        {
            std::visit([&](auto&& key) {
                const auto result = db.Search_Key(key, CMemory_Database::operation::KEY_EQUALS);
                const auto values = result.getValues();
                if (values.size() == 0)
                {
                    out << "ERROR" << std::endl;
                    out << "No values is valid type" << std::endl;
                    return;
                }
                for (const auto& value : values)
                {
                    std::visit([&](auto&& val) {
                        using T = std::decay_t<decltype(val)>;
                        if constexpr (std::is_arithmetic_v<T>)
                        {
                            max = std::max(max, static_cast<double>(val));
                        }
                        }, value);
                }
                out << "OK" << std::endl;
                out << result.getLines() << " rows." << std::endl;
                out << "MAX - " << max << std::endl;
                }, args[0]);
            return;
        }
    }
    msg += "Wrong number of arguments for ";
    msg += directiv_to_str(com.getDirectiv());
    msg += " expected max 1 got ";
    msg += std::to_string(args.size());
    throw std::runtime_error(msg);

    default:
        msg += "Unknow command";
        throw std::runtime_error(msg);
    }
}

int main(int argc, char** argv)
{
    CMemory_Database database;
    if (argc < 3)
    {
        std::cout << "Welcome in KIV/CPP semestral work - memory database.\n"
            "You can also run commands from file via runnig program with this set of arguments: <inputfile> <outputfile>\n"
            "Please, enter your query after this (>) symbol." << std::endl;
        try {
        auto com = loadCommand();
        while (com.getDirectiv() != EXIT)
        {
            execute_command(std::cout, database, com);
            com = loadCommand();
        }
        }
        catch (std::runtime_error& e)
        {
            std::cerr << "ERROR" << std::endl << e.what() << std::endl << "exiting" << std::endl;
            return EXIT_FAILURE;
        }
    } else
    {
        std::fstream in;
        std::fstream out;

        fspath input = std::string(argv[1]);
        fspath output = std::string(argv[2]);

        in.open(input.make_preferred().string(), std::ios::in);
        out.open(output.make_preferred().string(), std::ios::trunc | std::ios::out);

        if (!in.is_open())
        {
            std::cerr << "Can not open input file " << input.make_preferred().string() << " error:" << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
        if (!out.is_open())
        {
            std::cerr << "Can not open input file " << output.make_preferred().string() << " error:" << strerror(errno) << std::endl;
            return EXIT_FAILURE;
        }
        try {
            while (in)
            {
                auto com = parseCommand(in);
                if (com.getDirectiv() == UNKNOWN)
                {
                    continue;
                }
                out << com << std::endl;
                execute_command(out, database, com);
            }
        }
        catch (std::runtime_error& e)
        {
            std::cerr << e.what() << std::endl << "exiting" << std::endl;
            out << "ERROR" << std::endl << e.what();
            return EXIT_FAILURE;
        }
        std::cout << "Done" << std::endl;
    }
    return 0;
}
