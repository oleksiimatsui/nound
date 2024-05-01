
#pragma once
#include <JuceHeader.h>
#include "ValueRef.h"

class ValueRefComponent : public juce::Component, public ValueRef
{
public:
    ValueRefComponent(ValueRef *vr) : value_ref(vr){};

    ~ValueRefComponent() override
    {
    }

    void setComponent(Component *c)
    {
        component = c;
        addAndMakeVisible(component);
    }
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

    void resized() override
    {
        component->setBounds(getLocalBounds());
    }

protected:
    ValueRef *value_ref;
    Component *component;
};