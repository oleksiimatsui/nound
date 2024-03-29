#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "NodeTypes.h"
#include "PinComponent.h"


class NodeComponent : public juce::Component {
public:
    bool selected = false;
    juce::Point<int> position;
    std::vector<PinComponent*> inputs;
    std::vector<PinComponent*> outputs;


    NodeComponent(juce::Point<int> _position, Node * _node)
    {
        node = _node;
        theme = ThemeProvider::getCurrentTheme();
        
        for (auto& p : node->inputs) {
            auto pin = new PinComponent(&p);
            inputs.push_back(pin);
        }
        for (auto& p : node->outputs) {
            auto pin = new PinComponent(&p);
            outputs.push_back(pin);
        }
        for (auto& p : inputs) {
            addAndMakeVisible(p, 10);
        }
        for (auto& p : outputs) {
            addAndMakeVisible(p, 10);
        }
        position = _position;

        height = header_height + theme->pinSpacing*(node->outputs.size()+1)+theme->pinSpacing*(node->inputs.size()+1);

        setTransform(juce::AffineTransform::translation(_position));
        setSize(theme->nodeWidth, height);
    }

    ~NodeComponent() override
    {
        for (auto& n : inputs) delete n;
        inputs.clear();
        for (auto& n : outputs) delete n;
        outputs.clear();
    }

    void paint(juce::Graphics& g) {
        g.setColour((theme->nodeColor));
        g.fillRoundedRectangle(getX()+theme->pinDiameter/2, getY(),theme->nodeWidth-theme->pinDiameter, height, theme->nodeRounding);
        
        //draw header
        g.setColour((theme->nodeHeaderColor));
        juce::Path p;
        p.addRoundedRectangle (
            getX()+theme->pinDiameter/2,
            getY(),
            theme->nodeWidth-theme->pinDiameter,
            header_height,
            theme->nodeRounding,
            theme->nodeRounding,
            true,
            true,
            false,
            false);
        g.fillPath (p);
        g.setColour((theme->nodeTextColor));
        g.drawText(node->header, getLocalBounds(), juce::Justification::centredTop, true);
        
        if(selected){
            g.setColour((theme->selectedNodeBorderColor));
            g.drawRoundedRectangle(getX()+theme->pinDiameter/2, getY(),theme->nodeWidth-theme->pinDiameter, height, theme->nodeRounding, theme->selectedNodeBorderWidth);
        }
    }

    void resized() override {
        int margin = header_height;
        for (auto& p : outputs) {
            margin += theme->pinSpacing;
            p->setBounds(theme->nodeWidth - theme->pinDiameter, margin, theme->pinDiameter, theme->pinDiameter);
        }
        for (auto& p : inputs) {
            margin += theme->pinSpacing;
            p->setBounds(0, margin, theme->pinDiameter, theme->pinDiameter);
        }

    }

private:
    Theme * theme;
    Node * node;
    int header_height = 20;
    int height = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeComponent);
};

