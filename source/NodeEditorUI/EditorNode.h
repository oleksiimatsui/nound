#pragma once
#include <JuceHeader.h>
#include "NodeGraph.h"
#include "ValueRef.h"
#include <SettableComponent.h>

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

    void registerInput(int key, const std::string &name, int type, ValueRefComponent *c)
    {
        Node::registerInput(key, name, type);
        if (c != nullptr)
            input_components[key] = c;
    }

    void registerInput(int key, const std::string &name, int type)
    {
        Node::registerInput(key, name, type);
    }

    void registerInternal(ValueRefComponent *c)
    {
        internal_components.push_back(c);
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
    juce::Component *getInternal()
    {
        if (internal_components.size() == 0)
            return nullptr;
        return internal_components[0];
    };

    int x, y;
    int type_id;
    std::map<int, ValueRefComponent *> input_components;
    std::vector<ValueRefComponent *> internal_components;
    // std::unique_ptr<Vertical> internal_component;
};
