#pragma once
#include <JuceHeader.h>
#include "NodeGraph.h"

class EditorNode : public Node
{
public:
    EditorNode() : Node(){

                   };
    ~EditorNode()
    {
        for (auto &[_, n] : internals)
            delete n;
        internals.clear();
    }

    virtual juce::Component *getInternal()
    {
        return nullptr;
    };

    void registerInputWithComponent(int key, const std::string &name, PinType type, juce::Component *c)
    {
        registerInput(key, name, type);
        internals[key] = c;
    }

    juce::Component *getInternal(int key)
    {
        return internals[key];
    }

private:
    std::map<int, juce::Component *> internals;
};