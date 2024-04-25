#pragma once
#include "string"
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "NodeEditorComponent.h"
#include "NodeDropdown.h"
#include "NodeGraph.h"
#include "GraphProvider.h"

class StretchComponent : public juce::Component
{
public:
    StretchComponent(bool is_vertical, juce::Component *_A, juce::Component *_B, float proportion) : A(_A), B(_B), resizerBar(&stretchableManager, 1, vertical)
    {
        vertical = is_vertical;
        addAndMakeVisible(resizerBar);
        addAndMakeVisible(A);
        addAndMakeVisible(B);
        stretchableManager.setItemLayout(0,
                                         -0.01, -0.9,
                                         -proportion);
        stretchableManager.setItemLayout(1,
                                         5, 5, 5);
        stretchableManager.setItemLayout(2,
                                         -0.1, -0.9,
                                         -(1 - proportion));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(4);
        Component *comps[] = {A, &resizerBar, B};
        stretchableManager.layOutComponents(comps, 3,
                                            r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                            vertical, true);
    }

private:
    bool vertical;
    juce::Component *A;
    juce::Component *B;
    juce::StretchableLayoutManager stretchableManager;
    juce::StretchableLayoutResizerBar resizerBar;
};

class MainComponent : public juce::Component
{
public:
    MainComponent() : node_editor(g), stretcher(false, &dropdown_panel, &node_editor, 0.1)
    {
        setTitle("Welcome to nound");
        setDescription("sound node editor");
        setFocusContainerType(FocusContainerType::focusContainer);
        addAndMakeVisible(stretcher);

        file_button.setButtonText("FILE");
        file_button.onClick = [&]
        {
            juce::PopupMenu menu;
            menu.addItem("New", nullptr);
            menu.addItem("Open", nullptr);
            menu.addItem("Save", nullptr);
            menu.addItem("Export", nullptr);
            menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(file_button));
        };

        play_button.setButtonText("PLAY");
        play_button.onClick = [this]
        { play(); };
        stop_button.setButtonText("STOP");
        stop_button.onClick = [this]
        {
            stop();
        };
        toolbar.addElements(std::vector<juce::Component *>({&file_button, &play_button, &stop_button}));
        addAndMakeVisible(toolbar);
        setSize(500, 500);
    }
    ~MainComponent() override
    {
    }

    void paint(juce::Graphics &g)
    {
    }

    void play()
    {
        auto graph = GraphProvider::getGraph();
        Data d = nullptr;
        for (auto &[id, n] : graph->getNodes())
        {
            if (graph->getInputConnectionsOfNode(id).size() == 0)
            {
                n->trigger(d, nullptr);
            }
        };
        for (auto &[id, n] : graph->getNodes())
        {
            if (auto out = dynamic_cast<OutputNode *>(n))
            {
                player.setSource(out->result);
                player.Start();
            }
        };
    }
    void stop()
    {
        auto graph = GraphProvider::getGraph();
        player.Stop();
    }

    void resized() override
    {
        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::column;

        fb.items.add(juce::FlexItem(getWidth(), 50, toolbar));
        juce::FlexItem second_item(getWidth(), 50, stretcher);
        second_item = second_item.withFlex(1);
        fb.items.add(second_item);

        fb.performLayout(getLocalBounds());
    }

private:
    SoundOutputSource player;
    Graph *g = GraphProvider::getGraph();
    NodeEditorComponent node_editor;
    DropdownComponent dropdown_panel;
    StretchComponent stretcher;
    juce::TextButton play_button;
    juce::TextButton stop_button;
    juce::TextButton file_button;

    class Toolbar : public juce::Component
    {
    public:
        Toolbar()
        {
        }
        void addElements(std::vector<juce::Component *> _elements)
        {
            elements = _elements;
            for (auto &e : elements)
            {
                addAndMakeVisible(e);
            }
        }
        void resized() override
        {
            juce::FlexBox fb;
            fb.flexDirection = juce::FlexBox::Direction::row;
            for (auto &e : elements)
            {
                fb.items.add(juce::FlexItem(100, getHeight(), *e));
            }
            fb.performLayout(getLocalBounds());
        }
        std::vector<juce::Component *> elements;
    };
    Toolbar toolbar;
    std::vector<juce::Component *> elements;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
