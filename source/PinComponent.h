#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "NodeTypes.h"


class PinComponent : public juce::Component {
    public:
        PinComponent(Pin * _pin) {
            pin = _pin;
            theme = ThemeProvider::getCurrentTheme();
            setSize(theme->pinDiameter, theme->pinDiameter); // Set the size of the child component
        }
        void paint(juce::Graphics& g)  override {
            juce::Path p;
            p.addEllipse(0, 0, theme->pinDiameter,theme->pinDiameter);
            g.setColour(pin->type==PinType::Control ?  theme->controlPinColor : theme->soundPinColor);
            g.fillPath(p);
        }
        void resized() {
            setSize(theme->pinDiameter, theme->pinDiameter);
        }
    private:
        Theme * theme;
        Pin * pin;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
    };
