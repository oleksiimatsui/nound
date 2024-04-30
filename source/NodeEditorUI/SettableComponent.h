
#pragma once
#include <JuceHeader.h>
#include "ValueRef.h"

class SettableComponent : public juce::Component, public ValueRefParser
{
public:
    SettableComponent(ValueRefParser *vr) : value_ref(vr){};

    ~SettableComponent() override
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
    ValueRefParser *value_ref;
    Component *component;
};