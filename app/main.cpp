#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <regex>

#include <Mem_DB/CMemory_Database.hpp>


std::regex command_regex("(?<name>[A-Z_]+)\\((?<args>.+)\\)"); 
std::regex args_regex("[^,\\s][^\\,\\n]*[^,\\s]*");

std::regex double_regex("\\d?\\.\\d?");
std::regex string_regex("\".*\"");
std::regex pair_regex("\\[(?<Key>.+):(?<value>.+)\\]");

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
constexpr auto COMMANDS = { INSERT,DELETE,KEY_EQUALS,KEY_GREATER,KEY_LESS,FIND_VALUE,AVERAGE,MIN,MAX };

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

    Command(const Command& o) {
        directiv = o.directiv;
        args = o.args;
    };
    Command(Command&& o) {
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

    ~Command(){};

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
    auto bracket = line.find('(');
    std::string_view commnad = line.substr(0, bracket);
    for (auto& com : COMMANDS)
    {
        if (commnad.compare(command_to_str(com)) == 0)
        {
            return { com,line.substr(bracket) };
        }
    }
    if (commnad.compare(command_to_str(EXIT)) == 0)
    {
        return { EXIT,"" };
    }
    return { UNKNOWN,"" };
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << ">";
        Command com = parseCommand(std::cin);
        while (com.getDirectiv() != EXIT)
        {
            std::cout << com << std::endl;
            std::cout << ">";
            com = parseCommand(std::cin);
        }
    } else
    {
        std::string in = argv[1];

    }


    return 0;
}
