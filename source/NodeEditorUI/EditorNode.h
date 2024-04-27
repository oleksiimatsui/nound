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

    virtual void assignInputs(std::map<int, Value> values)
    {
    }
    virtual void assignInternals(std::vector<Value> values)
    {
    }
    virtual std::map<int, Value> getInputs()
    {
        return std::map<int, Value>();
    }
    virtual std::vector<Value> getInternals()
    {
        return std::vector<Value>();
    }

    // void linkCreated(int pinKey)
    // {
    //     values[pinKey].push_back(generateData());
    // }
    // void linkRemoved(int pinKey){
    //     values[pinKey].pop_back();
    // }

    // virtual Value generateData()=0;
    int x, y;
    int type_id;

protected:
    std::map<int, juce::Component *> internals;
    std::map<int, std::vector<Value>> values;
};