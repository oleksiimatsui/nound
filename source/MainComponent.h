#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "NodeEditorComponent.h"
class AppState
{
public:
    static AppState& getInstance()
    {
        static AppState instance;
        return instance;
    }
    bool isPlaying() const { return stateTree.getProperty("isPlaying", false); }
    void setPlaying(bool playing) { stateTree.setProperty("isPlaying", playing, nullptr); }
private:
    juce::ValueTree stateTree;
    AppState() { 
        stateTree = juce::ValueTree("Foo");
        stateTree.setProperty("isPlaying", false, nullptr); 
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppState)
};


class TreeViewComponent final : public juce::Component
{
public:
    TreeViewComponent()
    {
        //tree.setRootItem(&root);
        //tree.setRootItemVisible(false);
        //addAndMakeVisible(tree);
        //RootItem* rootItem = new RootItem();
        //tree.setRootItem(rootItem);
        setVisible(true);
    }


    void resized() override
    {
        tree.setBounds(getLocalBounds());
    }

private:
    //==============================================================================
    struct RootItem final : public juce::TreeViewItem
    {
        RootItem()
        {
            struct Item final : public TreeViewItem
            {
                Item(int index, int depth, int numSubItems)
                    : textToDisplay("Item " + juce::String(index)
                        + ". Depth: " + juce::String(depth)
                        + ". Num sub-items: " + juce::String(numSubItems))
                {
                    for (int i = 0; i < numSubItems; ++i)
                        addSubItem(new Item(i,
                            depth + 1,
                            juce::Random::getSystemRandom().nextInt(juce::jmax(0, 5 - depth))));
                }

                bool mightContainSubItems() override
                {
                    return getNumSubItems() > 0;
                }

                void paintItem(juce::Graphics& g, int width, int height) override
                {
                    if (isSelected())
                    {
                        g.setColour(juce::Colours::yellow.withAlpha(0.3f));
                        g.fillRect(0, 0, width, height);
                    }

                    g.setColour(juce::Colours::black);
                    g.drawRect(0, height - 1, width, 1);

                    g.setColour(juce::Colours::white);
                    g.drawText(textToDisplay, 0, 0, width, height, juce::Justification::centredLeft);
                }

                juce::String getAccessibilityName() override { return textToDisplay; }

                const juce::String textToDisplay;
            };

           /* for (int i = 0; i < 2; ++i)
                addSubItem(new Item(i, 0, juce::Random::getSystemRandom().nextInt(10)));*/
        }

        bool mightContainSubItems() override
        {
            return true;
        }
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RootItem)
    };

    //==============================================================================
    juce::TreeView tree;
    RootItem root;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TreeViewComponent)
};


class SoundResult final : public juce::Component {
public:
    SoundResult()
    {
        addAndMakeVisible(&play_button);
        play_button.setButtonText("PLAY");
        play_button.onClick = [this] { toggleAudio(); };
        setSize(300, 100);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(20.0f));
        g.drawRoundedRectangle(getLocalBounds().reduced(2).toFloat(), 5.0f, 3.0f);
    }

    void resized() override
    {
        play_button.setBounds(0, 0, 100, 50);
    }

    void toggleAudio() {
        bool isPlaying = AppState::getInstance().isPlaying();
        if (isPlaying)
        {
            AppState::getInstance().setPlaying(false);
        }
        else
        {
            AppState::getInstance().setPlaying(true);
        }
    }


private:
    juce::TextButton play_button;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundResult);
};


class NodeEditorWithPanel final : public juce::Component {
public:
    NodeEditorWithPanel()
    {
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

    void resized() override
    {
        auto r = getLocalBounds().reduced(4);
        Component* comps[] = { &tree, &resizerBar, &editor };
        stretchableManager.layOutComponents(comps, 3,
            r.getX(), r.getY(), r.getWidth(), r.getHeight(),
            false, true);
    }
private:

 //   juce::Viewport viewPort;
    NodeEditorComponent editor;
    TreeViewComponent  tree;
    juce::StretchableLayoutManager stretchableManager;
    juce::StretchableLayoutResizerBar resizerBar{ &stretchableManager, 1, true };
};


enum TransportState
{
    Stopped,
    Starting,
    Playing,
    Stopping
};

class MainComponent  : public juce::AudioAppComponent
{
public:
    MainComponent() {
        if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
            && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
        {
            juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                [&](bool granted) { setAudioChannels(granted ? 0 : 0, 0); });
        }
        else
        {
            setAudioChannels(0,0);
        }

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
    ~MainComponent() override {
        shutdownAudio();
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        random.setSeed(juce::Time::currentTimeMillis());
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        bool isPlaying = AppState::getInstance().isPlaying();

      /*  if (isPlaying)
        {
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            {
                auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                    buffer[sample] = random.nextFloat() * 0.25f - 0.125f;
            }
        }
        else {
            bufferToFill.clearActiveBufferRegion();
        }*/
    }
    void releaseResources() override {

    }
    void paint(juce::Graphics& g) {

    }
    void resized() override {
        auto r = getLocalBounds().reduced(4);

        Component* comps[] = { &sound_result, &resizerBar, &node_editor };

        stretchableManager.layOutComponents(comps, 3,
            r.getX(), r.getY(), r.getWidth(), r.getHeight(),
            true, true);
    }


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
        SoundResult sound_result;
    TreeViewComponent  tree;
    NodeEditorWithPanel node_editor;
    juce::StretchableLayoutManager stretchableManager;
    juce::StretchableLayoutResizerBar resizerBar{ &stretchableManager, 1, false };
    juce::Random random;
};

