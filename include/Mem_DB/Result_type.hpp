
#include <Mem_DB/p_types.hpp>

class Record {
public:
    Record(const DB_variant& key, const std::vector<DB_variant>& vals) noexcept :key(key), values(vals) {};
    Record(const DB_variant& key, const DB_variant& value) noexcept :key(key), values({})
    {
        values.push_back(value);
    };

    void add(const DB_variant& value) noexcept
    {
        values.push_back(value);
    }
    void add(const std::vector<DB_variant> values) noexcept
    {
        std::copy(values.begin(), values.end(), this->values.begin());
    }

    std::vector<DB_variant> getValues() const noexcept
    {
        return values;
    }

    friend std::ostream& operator<<(std::ostream& s, const Record& r) noexcept
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

class Result_type
{
public:
    Result_type() noexcept :valid(true), lines(0), records({}) {};
    Result_type(bool valid, size_t lines) noexcept :valid(valid), lines(lines), records({}) {};
    void add(const Record& rec) noexcept
    {
        records.push_back(rec);
        lines++;
    }
    void add(const DB_variant& key, const DB_variant& value) noexcept
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
    void add(const DB_variant& key, const std::vector<DB_variant>& values) noexcept
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

    inline size_t getLines() const noexcept
    {
        return lines;
    }

    inline bool empty() const noexcept
    {
        return records.size() == 0;
    }

    std::vector<DB_variant> getValues() const noexcept
    {
        std::vector<DB_variant> ret;
        for (auto& record : records)
        {
            const auto& vect = record.getValues();
            ret.insert(ret.end(), vect.begin(), vect.end());
        }
        return ret;
    }

    friend std::ostream& operator<<(std::ostream& s, const Result_type& r) noexcept
    {
        std::string t = r.valid ? "OK" : "ERROR";
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
    bool valid;
    size_t lines;
    std::vector<Record> records;
};


