#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/*
    entities:
        - attribute (with type and reverse index value->key)
        - values (to store list of attributes)
        - key-value-store (with mutex)
*/

enum class DataType : std::uint8_t
{
    DOUBLE,
    INTEGER,
    STRING,
    BOOLEAN
};

class TypeException : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "Data Type Error";
    }
};

struct KeyValuePair
{
    std::string key_;
    std::string value_;
};

class Attribute
{
public:
    Attribute(DataType type)
        : type_(type)
    {
    }

    DataType getType() const
    {
        return type_;
    }

    void addKey(const KeyValuePair& kvPair)
    {
        auto valueIter = valueToKey_.find(kvPair.value_);
        if (valueIter == valueToKey_.end())
            valueIter = valueToKey_.insert({kvPair.value_, {}}).first;

        valueIter->second.insert(kvPair.key_);
    }

    void removeKey(const KeyValuePair& kvPair)
    {
        auto valueIter = valueToKey_.find(kvPair.value_);
        if (valueIter == valueToKey_.end())
            return;

        valueIter->second.erase(kvPair.key_);
    }

    std::vector<std::string> searchByValue(const std::string& value)
    {
        auto iter = valueToKey_.find(value);
        if (iter == valueToKey_.end())
            return {};

        return {iter->second.begin(), iter->second.end()};
    }

private:
    std::unordered_map<std::string, std::set<std::string>> valueToKey_;
    DataType type_{};
};

using HAttribute = std::shared_ptr<Attribute>;

class Values
{
public:
    const std::unordered_map<std::string, std::string>& getAttributes() const
    {
        return attributes_;
    }

    void updateAttribute(const std::string& key, const std::string& value)
    {
        attributes_[key] = value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Values& v);

private:
    std::unordered_map<std::string, std::string> attributes_;
};

using HValues = std::shared_ptr<Values>;

std::ostream& operator<<(std::ostream& os, const Values& v)
{
    int count{};
    for (const auto& attribute : v.getAttributes())
    {
        if (count > 0)
            os << ", ";

        os << attribute.first << ": " << attribute.second;
        count++;
    }

    return os;
}

class RedisClone
{
public:
    HValues get(const std::string& key) const
    {
        std::shared_lock<std::shared_mutex> lock(rw_mutex);

        auto iter = keyValueStore_.find(key);
        if (iter == keyValueStore_.end())
            return nullptr;

        return iter->second;
    }

    void deleteKey(const std::string& key)
    {
        std::unique_lock<std::shared_mutex> lock(rw_mutex);

        auto iter = keyValueStore_.find(key);
        if (iter == keyValueStore_.end())
            return;

        const auto& values = iter->second;
        for (const auto& a : values->getAttributes())
        {
            const std::string& attKey = a.first;
            auto& attribute = attributes_.at(attKey);
            attribute->removeKey({.key_ = key, .value_ = a.second});
        }

        keyValueStore_.erase(iter);
    }

    std::vector<std::string> getKeys() const
    {
        std::shared_lock<std::shared_mutex> lock(rw_mutex);

        std::vector<std::string> keys(keyValueStore_.size());
        for (const auto& elem : keyValueStore_)
            keys.push_back(elem.first);

        return keys;
    }

    void put(const std::string& key,
             const std::vector<std::pair<std::string, std::string>>& attributes)
    {
        std::unique_lock<std::shared_mutex> lock(rw_mutex);

        // preprocess errors in data types
        for (const auto& kvPair : attributes)
        {
            const auto& attKey = kvPair.first;
            const auto& attValue = kvPair.second;

            DataType recdDataType = determineType(attValue);
            auto attIter = attributes_.find(attKey);
            if (attIter != attributes_.end())
            {
                DataType expectedType = attIter->second->getType();
                if (recdDataType != expectedType)
                    throw TypeException();
            }
            else
            {
                attributes_.insert({attKey, std::make_shared<Attribute>(recdDataType)});
            }
        }

        auto iter = keyValueStore_.find(key);
        if (iter == keyValueStore_.end())
            iter = keyValueStore_.insert({key, std::make_shared<Values>()}).first;

        HValues& values = iter->second;

        for (const auto& kvPair : attributes)
        {
            const auto& attKey = kvPair.first;
            const auto& attValue = kvPair.second;

            auto& attribute = attributes_.at(attKey);

            auto prevValueIter = values->getAttributes().find(attKey);
            if (prevValueIter != values->getAttributes().end())
                attribute->removeKey({.key_ = key, .value_ = prevValueIter->second});

            attribute->addKey({.key_ = key, .value_ = attValue});
            values->updateAttribute(attKey, attValue);
        }
    }

    std::vector<std::string> search(const KeyValuePair& kvPair) const
    {
        std::shared_lock<std::shared_mutex> lock(rw_mutex);

        auto attIter = attributes_.find(kvPair.key_);
        if (attIter == attributes_.end())
            return {};

        return attIter->second->searchByValue(kvPair.value_);
    }

private:
    std::map<std::string, HValues> keyValueStore_;
    std::unordered_map<std::string, HAttribute> attributes_;
    mutable std::shared_mutex rw_mutex;

    static DataType determineType(const std::string& s)
    {
        if (s == "true" || s == "false")
            return DataType::BOOLEAN;

        bool has_digit{false};
        bool has_dot{false};
        bool valid_number{true};

        for (size_t i = 0; i < s.size(); i++)
        {
            if (i == 0 && s[i] == '-')
                continue;

            if (s[i] == '.')
            {
                if (has_dot)
                {
                    valid_number = false;
                    break;
                }

                has_dot = true;
            }
            else if (std::isdigit(s[i]) != 0)
            {
                has_digit = true;
            }
            else
            {
                valid_number = false;
                break;
            }
        }

        if (valid_number && has_digit)
        {
            if (has_dot)
                return DataType::DOUBLE;

            return DataType::INTEGER;
        }

        return DataType::STRING;
    }
};

using HRedisClone = std::unique_ptr<RedisClone>;

int main()
{
    std::cout << "Welcome to RedisClone!\n";
    HRedisClone redisClone = std::make_unique<RedisClone>();

    std::string line{};

    while (std::getline(std::cin, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string cmd{};
        ss >> cmd;

        if (cmd == "exit")
            break;
        else if (cmd == "get")
        {
            std::string key{};
            ss >> key;

            auto val = redisClone->get(key);

            if (!val)
                std::cout << "No entry found for " << key << '\n';
            else
                std::cout << *val << '\n';
        }
        else if (cmd == "keys")
        {
            auto keys = redisClone->getKeys();
            int count{};
            for (const auto& k : keys)
            {
                if (count > 0)
                    std::cout << ',';

                std::cout << k;
                count++;
            }
            std::cout << '\n';
        }
        else if (cmd == "delete")
        {
            std::string key{};
            ss >> key;

            redisClone->deleteKey(key);
        }
        else if (cmd == "search")
        {
            std::string attKey{};
            std::string attVal{};
            ss >> attKey >> attVal;

            auto keys = redisClone->search({.key_ = attKey, .value_ = attVal});
            int count{};
            for (const auto& k : keys)
            {
                if (count > 0)
                    std::cout << ',';

                std::cout << k;
                count++;
            }
            std::cout << '\n';
        }
        else if (cmd == "put")
        {
            std::string key{};
            ss >> key;

            std::vector<std::pair<std::string, std::string>> attrs{};
            std::string attrK{};
            std::string attrV{};
            while (ss >> attrK >> attrV)
            {
                attrs.emplace_back(attrK, attrV);
            }

            try
            {
                redisClone->put(key, attrs);
            }
            catch (const TypeException& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }

    return 0;
}