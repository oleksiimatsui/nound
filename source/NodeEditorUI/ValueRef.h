#pragma once
#include "string"
#include "iostream"
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
        return value;
    }
    void fromString(const std::string &str) override
    {
        if (str == "nothing")
        {
            value = "";
        }
        value = str;
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