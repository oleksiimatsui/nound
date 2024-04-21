#pragma once
#include "string"
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "NodeEditorComponent.h"
#include "NodeDropdown.h"
#include "NodeGraph.h"
#include "GraphProvider.h"

class AppState
{
public:
    static AppState &getInstance()
    {
        static AppState instance;
        return instance;
    }
    bool isPlaying() const { return stateTree.getProperty("isPlaying", false); }
    void setPlaying(bool playing) { stateTree.setProperty("isPlaying", playing, nullptr); }

private:
    juce::ValueTree stateTree;
    AppState()
    {
        stateTree = juce::ValueTree("Foo");
        stateTree.setProperty("isPlaying", false, nullptr);
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppState)
};

class SoundResult final : public juce::Component
{
public:
    SoundResult()
    {
        addAndMakeVisible(&play_button);
        play_button.setButtonText("PLAY");
        play_button.onClick = [this]
        { play(); };
        addAndMakeVisible(&stop_button);
        stop_button.setButtonText("STOP");
        stop_button.onClick = [this]
        {
            stop();
        };
        setSize(300, 100);
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(20.0f));
        g.drawRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 5.0f, 3.0f);
    }

    void resized() override
    {
        play_button.setBounds(0, 0, 100, 50);
        stop_button.setBounds(100, 0, 100, 50);
    }

    void play()
    {
        auto graph = GraphProvider::getGraph();
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

private:
    Data d;
    juce::TextButton play_button;
    juce::TextButton stop_button;
    SoundOutputSource player;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundResult);
};

class NodeEditorWithPanel final : public juce::Component
{
public:
    NodeEditorWithPanel()
    {
        GraphProvider::reset();
        Graph *graph = GraphProvider::getGraph();
        editor = new NodeEditorComponent(graph);
        tree = new DropdownComponent();

        addAndMakeVisible(tree);
        addAndMakeVisible(resizerBar);
        addAndMakeVisible(editor);

        stretchableManager.setItemLayout(0,
                                         -0.05, -0.9,
                                         -0.1);
        stretchableManager.setItemLayout(1,
                                         5, 5, 5);
        stretchableManager.setItemLayout(2,
                                         -0.1, -0.9,
                                         -0.9);
    }

    ~NodeEditorWithPanel()
    {
        delete editor;
        delete tree;
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(4);
        Component *comps[] = {tree, &resizerBar, editor};
        stretchableManager.layOutComponents(comps, 3,
                                            r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                            false, true);
    }

private:
    //   juce::Viewport viewPort;
    NodeEditorComponent *editor;
    DropdownComponent *tree;
    juce::StretchableLayoutManager stretchableManager;
    juce::StretchableLayoutResizerBar resizerBar{&stretchableManager, 1, true};
};

enum TransportState
{
    Stopped,
    Starting,
    Playing,
    Stopping
};

class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        setTitle("Welcome to nound");
        setDescription("sound node editor");
        setFocusContainerType(FocusContainerType::focusContainer);

        addAndMakeVisible(sound_result);
        addAndMakeVisible(resizerBar);
        addAndMakeVisible(node_editor);

        stretchableManager.setItemLayout(0,
                                         -0.01, -0.9,
                                         -0.1);
        stretchableManager.setItemLayout(1,
                                         5, 5, 5);
        stretchableManager.setItemLayout(2,
                                         -0.1, -0.9,
                                         -0.9);

        setSize(500, 500);
    }
    ~MainComponent() override
    {
    }

    void paint(juce::Graphics &g)
    {
    }
    void resized() override
    {
        auto r = getLocalBounds().reduced(4);

        Component *comps[] = {&sound_result, &resizerBar, &node_editor};

        stretchableManager.layOutComponents(comps, 3,
                                            r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                            true, true);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    SoundResult sound_result;
    NodeEditorWithPanel node_editor;
    juce::StretchableLayoutManager stretchableManager;
    juce::StretchableLayoutResizerBar resizerBar{&stretchableManager, 1, false};
    juce::Random random;
};
