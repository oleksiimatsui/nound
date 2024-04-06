#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "NodeTypes.h"
#include "EditorNode.h"
#include "PinComponent.h"

class NodeComponent : public juce::Component
{
public:
    bool selected = false;
    juce::Point<int> position;
    std::vector<PinComponent *> inputs;
    std::vector<PinComponent *> outputs;
    std::vector<Label *> inputNames;
    std::vector<Label *> outputNames;

    NodeComponent(juce::Point<int> _position, EditorNode *_node)
    {
        node = _node;
        theme = ThemeProvider::getCurrentTheme();
        f.setHeight(theme->nodeTextHeight);

        for (auto &[_, p] : node->inputs)
        {
            auto pin = new PinComponent(&p);
            inputs.push_back(pin);
            auto label = new Label({}, p.name);
            label->setFont(f);
            label->setColour(juce::Label::textColourId, theme->nodeTextColor);
            inputNames.push_back(label);
        }
        for (auto &[_, p] : node->outputs)
        {
            auto pin = new PinComponent(&p);
            outputs.push_back(pin);
            auto label = new Label({}, p.name);
            label->setFont(f);
            label->setColour(juce::Label::textColourId, theme->nodeTextColor);
            outputNames.push_back(label);
        }
        for (auto &p : inputNames)
        {
            p->setJustificationType(Justification::centredLeft);
            p->setBounds(80, 80, theme->nodeWidth, 100);
            p->setInterceptsMouseClicks(false, false);
            addAndMakeVisible(p);
        }
        for (auto &p : outputNames)
        {
            p->setJustificationType(Justification::centredRight);
            p->setBounds(80, 80, theme->nodeWidth, 100);
            p->setInterceptsMouseClicks(false, false);
            addAndMakeVisible(p);
        }
        for (auto &p : inputs)
        {
            addAndMakeVisible(p, 10);
        }
        for (auto &p : outputs)
        {
            addAndMakeVisible(p, 10);
        }

        internal = node->getInternal();
        // internal->setBounds(80, 80, theme->nodeWidth, 100);
        addAndMakeVisible(internal);

        position = _position;

        spacing = theme->pinDiameter * 2;
        height = internal->getHeight() + spacing + spacing + spacing * (node->outputs.size()) + spacing * (node->inputs.size());

        setTransform(juce::AffineTransform::translation(_position));
        setSize(theme->nodeWidth, height);
    }

    ~NodeComponent() override
    {
        for (auto &n : inputs)
            delete n;
        inputs.clear();
        for (auto &n : outputs)
            delete n;
        outputs.clear();
        for (auto &n : outputNames)
            delete n;
        outputNames.clear();
        for (auto &n : inputNames)
            delete n;
        inputNames.clear();
        delete internal;
    }

    void paint(juce::Graphics &g)
    {
        g.setColour((theme->nodeColor));
        g.fillRoundedRectangle(getX() + theme->pinDiameter / 2, getY(), theme->nodeWidth - theme->pinDiameter, height, theme->nodeRounding);
        auto f = juce::Font();
        // draw header
        g.setColour((theme->nodeHeaderColor));
        juce::Path p;
        p.addRoundedRectangle(
            getX() + theme->pinDiameter / 2,
            getY(),
            theme->nodeWidth - theme->pinDiameter,
            spacing,
            theme->nodeRounding,
            theme->nodeRounding,
            true,
            true,
            false,
            false);
        g.fillPath(p);
        g.setColour((theme->nodeTextColor));
        g.drawText(node->header, juce::Rectangle<int>(getX() + theme->pinDiameter / 2, getY(), theme->nodeWidth - theme->pinDiameter, spacing), juce::Justification::centred, true);

        if (selected)
        {
            g.setColour((theme->selectedNodeBorderColor));
            g.drawRoundedRectangle(getX() + theme->pinDiameter / 2, getY(), theme->nodeWidth - theme->pinDiameter, height, theme->nodeRounding, theme->selectedNodeBorderWidth);
        }
    }

    void resized() override
    {
        int margin = spacing - spacing * 0.5;
        for (auto &p : outputs)
        {
            margin += spacing;
            p->setBounds(theme->nodeWidth - theme->pinDiameter, margin - theme->pinDiameter / 2, theme->pinDiameter, theme->pinDiameter);
        }
        for (auto &p : inputs)
        {
            margin += spacing;
            p->setBounds(0, margin - theme->pinDiameter / 2, theme->pinDiameter, theme->pinDiameter);
        }
        margin = spacing - spacing * 0.5;
        for (auto &p : outputNames)
        {
            margin += spacing;
            p->setBounds(theme->padding, margin - spacing / 2, theme->nodeWidth - theme->padding * 2, spacing);
        }
        for (auto &p : inputNames)
        {
            margin += spacing;
            p->setBounds(theme->padding, margin - spacing / 2, theme->nodeWidth - theme->padding * 2, spacing);
        }

        margin += spacing;
        internal->setBounds(theme->padding, margin, theme->nodeWidth - theme->padding * 2, internal->getHeight());
    }

private:
    juce::Font f;
    Theme *theme;
    EditorNode *node;
    int height;
    int spacing;
    juce::Component *internal;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeComponent);
};
