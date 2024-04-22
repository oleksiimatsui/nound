#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "NodeTypes.h"

class PinComponent : public juce::Component
{
public:
    PinComponent(Pin *_pin)
    {
        pin = _pin;
        theme = ThemeProvider::getCurrentTheme();
        setSize(theme->pinDiameter, theme->pinDiameter); // Set the size of the child component
    }
    void paint(juce::Graphics &g) override
    {
        juce::Path p;
        p.addEllipse(0, 0, theme->pinDiameter, theme->pinDiameter);
        g.setColour(getColor());
        g.fillPath(p);
    }
    void resized()
    {
        setSize(theme->pinDiameter, theme->pinDiameter);
    }
    juce::Colour getColor()
    {
        juce::Colour res;
        switch (pin->type)
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
    Pin *pin;

private:
    Theme *theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
};
