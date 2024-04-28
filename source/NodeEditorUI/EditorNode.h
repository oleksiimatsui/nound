#pragma once
#include <JuceHeader.h>
#include "NodeGraph.h"

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
    }

    virtual juce::Component *getInternal()
    {
        return nullptr;
    };

    void registerInput(int key, const std::string &name, int type, juce::Component *c, Value *value)
    {
        Node::registerInput(key, name, type);
        input_components[key] = c;
        input_values[key] = value;
    }
    // void registerInput(int key, const std::string &name, int type, juce::Component *c)
    // {
    //     Node::registerInput(key, name, type);
    //     input_components[key] = c;
    // }
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

    virtual void assignInputs(std::map<int, Value> values)
    {
    }
    virtual void assignInternals(std::vector<Value> values)
    {
    }
    virtual std::map<int, Value> getInputValues()
    {
        std::map<int, Value> res;
        for (auto &[id, val] : input_values)
        {
            res[id] = &val;
        }
        return res;
    }
    virtual std::vector<Value> getInternalValues()
    {
        return std::vector<Value>();
    }

    int x, y;
    int type_id;

protected:
    std::map<int, juce::Component *> input_components;
    std::map<int, Value *> input_values;
};