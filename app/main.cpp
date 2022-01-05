#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>
#include <concepts>

#include <Mem_DB/CMemory_Database.hpp>

std::regex command_regex("([A-Z_]+)\\((.*)\\)", std::regex::ECMAScript);
constexpr size_t command_regex_command = 1;
constexpr size_t command_regex_args = 2;

std::regex args_regex("[^,\\s][^\\,\\n]*[^,\\s]*", std::regex::ECMAScript);

std::regex double_regex("\\d?\\.\\d?", std::regex::ECMAScript);
std::regex string_regex("\"[^\"]*\"", std::regex::ECMAScript);
//                        key   value
std::regex pair_regex("\\[(.+):(.+)\\]", std::regex::ECMAScript);
constexpr size_t pair_regex_key = 1;
constexpr size_t pair_regex_value = 2;



enum class Directiv {
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

constexpr  std::string_view command_to_str(const Directiv& com)
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

std::ostream& operator<<(std::ostream& s, const Directiv com)
{
    s << command_to_str(com);
    return s;
}

class Command {
public:
    Command(Directiv d, std::string&& args) :directiv(d), args(std::move(args)) {}

    Command(const Command& o)
    {
        directiv = o.directiv;
        args = o.args;
    };
    Command(Command&& o)
    {
        directiv = o.directiv; o.directiv = UNKNOWN;
        args = std::move(o.args);
    };


    Command& operator=(const Command& o)
    {
        directiv = o.directiv;
        args = o.args;
        return *this;
    }

    Command& operator=(Command&& o)
    {
        directiv = o.directiv; o.directiv = UNKNOWN;
        args = std::move(o.args);
        return *this;
    }

    ~Command() {};

    friend std::ostream& operator<<(std::ostream& s, Command& c)
    {
        s << "[" << c.directiv << " " << c.args << "]";
        return s;
    };

    inline Directiv getDirectiv()
    {
        return directiv;
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
        if (line.compare(command_to_str(EXIT)) == 0)
        {
            return { EXIT,"" };
        }
        return { UNKNOWN,"" };
    }
    auto command = match[command_regex_command].str();
    for (auto& com : COMMANDS)
    {
        if (command.compare(command_to_str(com)) == 0)
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

void doCommand(const Command& com)
{
    
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        auto com = loadCommand();
        while (com.getDirectiv() != EXIT)
        {
            std::cout << com << std::endl;
            com = loadCommand();
        }
    } else
    {
        std::string in = argv[1];

    }

    CMemory_Database database;

    std::string key = "pokus";
    std::cout << database.Insert(1,1,2,3,4,5);
    std::cout << database.Insert("1",5,4,3,2,1);
    std::cout << database.Insert(Pair(1, "auto"), 1, 2, 3, 4, 5, 6, 8, 9, 10);
    std::cout << database.Insert(Pair(1, 1), 1, 2, 3, 4, 5, 6, 8, 9, 10);
    std::cout << database.Insert(Pair(1,1),"hello");
    std::cout << database.Search_Key(1, CMemory_Database::DB_operation::KEY_EQUALS);
    std::cout << database.Search_Key("1", CMemory_Database::DB_operation::KEY_EQUALS);
    std::cout << database.Search_Key(1.6, CMemory_Database::DB_operation::KEY_EQUALS);
    std::cout << database.Search_Key(Pair(1, 1), CMemory_Database::DB_operation::KEY_EQUALS);

    std::cout << (Pair(1, 2) <=> Pair(1, 2) == std::strong_ordering::equal) << std::endl;
    std::cout << (Pair(1, 3) <=> Pair(1, 2) == std::strong_ordering::equal) << std::endl;
    std::cout << (Pair(2, 2) <=> Pair(1, 2) == std::strong_ordering::equal) << std::endl;
    std::cout << (Pair(1, 2) <=> Pair(1, 4) == std::strong_ordering::equal) << std::endl;
    std::cout << (Pair(1, 2) <=> Pair(2, 2) == std::strong_ordering::equal) << std::endl;
    std::cout << (Pair(1.0, 2.) <=> Pair("bla", 2.5) == std::strong_ordering::equal) << std::endl;

    return 0;
}
