
#pragma once
#include <JuceHeader.h>
#include "ValueRef.h"

class SettableComponent : public juce::Component, public ValueRef
{
public:
    SettableComponent(ValueRef *vr) : value_ref(vr){};
    std::string toString() override
    {
        return value_ref->toString();
    }
    void fromString(const std::string &str) override
    {
        value_ref->fromString(str);
        update();
    }
    virtual void update() = 0;

private:
    ValueRef *value_ref;
};