#pragma once
#include <JuceHeader.h>
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
    std::vector<juce::Label *> inputNames;
    std::vector<juce::Label *> outputNames;

    NodeComponent(juce::Point<int> _position, EditorNode *_node)
    {
        node = _node;
        theme = ThemeProvider::getCurrentTheme();
        f.setHeight(theme->nodeTextHeight);
        spacing = theme->pinDiameter * 2;
        height = 0;
        height += spacing * 2;

        for (auto &[_, p] : node->inputs)
        {
            auto pin = new PinComponent(p);
            inputs.push_back(pin);
            auto label = new juce::Label({}, p->name);
            label->setFont(f);
            label->setColour(juce::Label::textColourId, theme->nodeTextColor);
            inputNames.push_back(label);
            height += spacing;

            juce::Component *in = node->getInternal(p->key);
            if (in != nullptr)
            {
                addAndMakeVisible(in);
                height += in->getHeight();
            }
        }
        for (auto &[_, p] : node->outputs)
        {
            auto pin = new PinComponent(p);
            outputs.push_back(pin);
            auto label = new juce::Label({}, p->name);
            label->setFont(f);
            label->setColour(juce::Label::textColourId, theme->nodeTextColor);
            outputNames.push_back(label);
            height += spacing;
        }
        for (auto &p : inputNames)
        {
            p->setJustificationType(juce::Justification::centredLeft);
            p->setBounds(80, 80, theme->nodeWidth, 100);
            p->setInterceptsMouseClicks(false, false);
            addAndMakeVisible(p);
        }
        for (auto &p : outputNames)
        {
            p->setJustificationType(juce::Justification::centredRight);
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

        position = _position;

        // height = spacing + spacing + spacing * (node->outputs.size()) + spacing * (node->inputs.size() * 2);

        internal = node->getInternal();

        if (internal != nullptr)
        {
            height += internal->getHeight();
            addAndMakeVisible(internal);
        }

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
        int i = 0;
        int margin = spacing - spacing * 0.5;
        for (auto &p : outputs)
        {
            margin += spacing;
            p->setBounds(theme->nodeWidth - theme->pinDiameter, margin - theme->pinDiameter / 2, theme->pinDiameter, theme->pinDiameter);
            outputNames[i]->setBounds(theme->padding, margin - spacing / 2, theme->nodeWidth - theme->padding * 2, spacing);
            i++;
        }
        i = 0;
        margin += spacing;
        for (auto &p : inputs)
        {

            p->setBounds(0, margin - theme->pinDiameter / 2, theme->pinDiameter, theme->pinDiameter);
            auto label = inputNames[i];
            label->setBounds(theme->padding, margin - spacing / 2, theme->nodeWidth - theme->padding * 2, spacing);
            juce::Component *in = node->getInternal(p->pin->key);
            if (in != nullptr)
            {
                margin += spacing;
                in->setBounds(theme->padding, margin - spacing / 2, theme->nodeWidth - theme->padding * 2, in->getHeight());
                margin += in->getHeight();
            }
            else
            {
                margin += spacing;
            }

            i++;
        }

        if (internal == nullptr)
            return;

        internal->setBounds(theme->padding, margin, theme->nodeWidth - theme->padding * 2, internal->getHeight());
        margin += spacing;
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
