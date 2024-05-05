#pragma once
#include "string"
#include "iostream"
#include "algorithm"

void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

class ValueRef
{
public:
    virtual std::string toString() = 0;
    virtual void fromString(const std::string &str) = 0;
};

class StringRef : public ValueRef
{
public:
    std::string &value;
    StringRef(std::string &val) : value(val){};
    std::string toString() override
    {
        if (value == "")
        {
            return "nothing";
        }
        std::string res = value;
        replaceAll(res, " ", "%");
        return res;
    }
    void fromString(const std::string &str) override
    {
        if (str == "nothing")
        {
            value = "";
        }
        value = str;
        replaceAll(value, "%", " ");
    }
};
class IntRef : public ValueRef
{
public:
    int &value;
    IntRef(int &val) : value(val){};
    std::string toString() override
    {
        return std::to_string(value);
    }
    void fromString(const std::string &str) override
    {
        value = std::stoi(str);
    }
};
class FloatRef : public ValueRef
{
public:
    float &value;
    FloatRef(float &val) : value(val){

                           };
    std::string toString() override
    {
        return std::to_string(value);
    }
    void fromString(const std::string &str) override
    {
        value = std::stof(str);
    }
};