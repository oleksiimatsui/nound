#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "NodeTypes.h"

class ConnectionComponent : public juce::Component
{
public:
    ConnectionComponent(PinComponent *pin1, PinComponent *pin2)
    {
        theme = ThemeProvider::getCurrentTheme();
        pin_from = pin1;
        pin_to = pin2;
        selected = false;
    }
    void paint(juce::Graphics &g) override
    {
        auto w = static_cast<float>(getWidth());
        auto h = static_cast<float>(getHeight());
        juce::Point<float> start, end;
        juce::Path path;
        if (inverted)
        {
            path.startNewSubPath(0, h);
            path.cubicTo(w * 0.5f, h, w * 0.5f, 0, w, 0);
        }
        else
        {
            path.startNewSubPath(0, 0);
            path.cubicTo(w * 0.5f, 0, w * 0.5f, h, w, h);
        }
        auto t = juce::PathStrokeType(theme->connectionsThickness);
        g.setColour(selected ? theme->selectedConnectionColor : pin_from->getColor());
        g.strokePath(path, t);
    };
    void calculateBounds(const juce::Point<int> &start, const juce::Point<int> &end)
    {
        auto offset = end - start;
        auto w = abs(offset.x);
        auto h = abs(offset.y);
        auto up = offset.y < 0;
        auto down = offset.y >= 0;
        auto left = offset.x < 0;
        auto right = offset.x >= 0;

        w = (w > 1) ? w : 2;
        h = (h > 1) ? h : 2;

        inverted = (up && right) || (down && left);

        if (down && right)
        {
            setBounds(start.x, start.y, w, h);
        }
        else if (up && left)
        {
            setBounds(start.x - w, start.y - h, w, h);
        }
        else if (up && right)
        {
            setBounds(start.x, start.y - h, w, h);
        }
        else if (down && left)
        {
            setBounds(start.x - w, start.y, w, h);
        }
    }

    PinComponent *pin_from;
    PinComponent *pin_to;

    bool selected;

private:
    Theme *theme;
    bool inverted;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionComponent);
};

class ConnectionPreview : public juce::Component
{
public:
    ConnectionPreview()
    {
        theme = ThemeProvider::getCurrentTheme();
    }

    void paint(juce::Graphics &g) override
    {
        auto w = static_cast<float>(getWidth());
        auto h = static_cast<float>(getHeight());
        juce::Point<float> start, end;
        juce::Path path;

        if (inverted)
        {
            path.startNewSubPath(0, h);
            path.cubicTo(w * 0.5f, h, w * 0.5f, 0, w, 0);
        }
        else
        {
            path.startNewSubPath(0, 0);
            path.cubicTo(w * 0.5f, 0, w * 0.5f, h, w, h);
        }
        auto t = juce::PathStrokeType(theme->connectionsThickness);
        g.setColour(startPin->getColor());
        g.strokePath(path, t);
    }

    void calculateBounds(const juce::Point<int> &start, const juce::Point<int> &end)
    {
        auto offset = end - start;
        auto w = abs(offset.x);
        auto h = abs(offset.y);
        auto up = offset.y < 0;
        auto down = offset.y >= 0;
        auto left = offset.x < 0;
        auto right = offset.x >= 0;

        w = (w > 1) ? w : 2;
        h = (h > 1) ? h : 2;

        inverted = (up && right) || (down && left);

        if (down && right)
        {
            setBounds(start.x, start.y, w, h);
        }
        else if (up && left)
        {
            setBounds(start.x - w, start.y - h, w, h);
        }
        else if (up && right)
        {
            setBounds(start.x, start.y - h, w, h);
        }
        else if (down && left)
        {
            setBounds(start.x - w, start.y, w, h);
        }
    }

    PinComponent *startPin;
    juce::Point<int> currentEndPosition;

private:
    Theme *theme;
    bool inverted;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionPreview);
};