#pragma once
#include <JuceHeader.h>
#include "NodeGraph.h"
#include "ValueRef.h"

enum PinType
{
    Number,
    Function,
    Audio
};

class EditorNode : public Node
{
public:
    EditorNode() : Node()
    {
        x = 0;
        y = 0;
        type_id = 0;
    };
    ~EditorNode()
    {
        for (auto &[_, n] : input_components)
            delete n;
        input_components.clear();
        for (auto &[_, n] : input_values)
            delete n;
        input_values.clear();
    }

    virtual juce::Component *getInternal()
    {
        return nullptr;
    };

    void registerInput(int key, const std::string &name, int type, juce::Component *c, ValueRef *val)
    {
        Node::registerInput(key, name, type);
        input_components[key] = c;
        input_values[key] = val;
    }

    void registerInput(int key, const std::string &name, int type)
    {
        Node::registerInput(key, name, type);
    }
    juce::Colour getPinColor(int type)
    {
        auto theme = ThemeProvider::getCurrentTheme();
        juce::Colour res;
        switch (type)
        {
        case PinType::Number:
            res = theme->numberPinColor;
            break;
        case PinType::Function:
            res = theme->wavePinColor;
            break;
        case PinType::Audio:
            res = theme->soundPinColor;
            break;
        }
        return res;
    }

    juce::Component *getInternal(int key)
    {
        return input_components[key];
    }

    void assignInput(int key, std::string value)
    {

        ValueRef *vh = input_values[key];
        vh->fromString(value);
    }
    virtual std::string getInputValue(int key)
    {
        return input_values[key]->toString();
    }
    virtual void assignInternal(std::vector<Value> values)
    {
    }

    virtual std::string getInternalValue()
    {
        return "";
    }

    int x, y;
    int type_id;

protected:
    std::map<int, juce::Component *> input_components;
    std::map<int, ValueRef *> input_values;
};