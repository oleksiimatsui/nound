#pragma once
#include "string"
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "NodeEditorComponent.h"
#include "NodeDropdown.h"
#include "NodeGraph.h"
#include "GraphProvider.h"

class FlexWithColor : public juce::Component
{
public:
    FlexWithColor() : juce::Component()
    {
        drawBorder = false;
        padding = 0;
        margin = 0;
    }
    void setPadding(int p)
    {
        padding = p;
    }
    void setMargin(int m)
    {
        margin = m;
    }
    void setColor(juce::Colour _color)
    {
        color = _color;
    }
    void setBorder(juce::Colour _color)
    {
        border = _color;
        drawBorder = true;
    }
    void setDirection(juce::FlexBox::Direction j)
    {
        direction = j;
    }

    void setAlign(juce::FlexBox::AlignContent j)
    {
        align = j;
    }
    void setJustify(juce::FlexBox::JustifyContent j)
    {
        justify = j;
    }

    void addElements(std::vector<juce::Component *> _elements)
    {
        elements = _elements;
        for (auto &e : elements)
        {
            addAndMakeVisible(e);
        }
    }
    void paint(juce::Graphics &g) override
    {
        g.fillAll(color);
        if (drawBorder)
        {
            g.setColour(border);
            g.drawRect(getLocalBounds());
        }
    }
    void resized() override
    {
        juce::FlexBox fb;
        fb.justifyContent = justify;
        fb.alignContent = align;
        fb.flexDirection = direction;
        for (auto &e : elements)
        {
            fb.items.add(juce::FlexItem(100, getHeight() - padding * 2, *e).withMargin(margin));
        }
        fb.performLayout(getLocalBounds());
    }
    std::vector<juce::Component *> elements;
    juce::Colour color;
    juce::Colour border;
    bool drawBorder;
    juce::FlexBox::JustifyContent justify;
    juce::FlexBox::AlignContent align;
    juce::FlexBox::Direction direction = juce::FlexBox::Direction::row;
    int padding, margin;
};

class MenuButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (shouldDrawButtonAsDown)
            g.setColour(App::ThemeProvider::getCurrentTheme()->hoveredColor);
    }
};
class MenuButton : public juce::TextButton
{
public:
    MenuButton() : juce::TextButton()
    {
        setLookAndFeel(&lookAndFeel);
    }

    ~MenuButton()
    {
        setLookAndFeel(nullptr);
    }

private:
    MenuButtonLookAndFeel lookAndFeel;
};

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
        setTitle("Welcome to Nound");
        setDescription("Sound node editor");
        setFocusContainerType(FocusContainerType::focusContainer);
        addAndMakeVisible(stretcher);

        file_button.setButtonText("File");
        file_button.onClick = [&]
        {
            juce::PopupMenu menu;
            menu.addItem("New", [this]
                         { new_graph(); });
            menu.addItem("Open", [this]
                         { open(); });
            menu.addItem("Save", [this]
                         { save(); });
            menu.addItem("Export", [this]
                         { export_graph(); });
            menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(file_button));
        };
        toolbar.setColor(App::ThemeProvider::getCurrentTheme()->darkerColor);
        toolbar.addElements(std::vector<juce::Component *>({&file_button}));
        addAndMakeVisible(toolbar);

        play_panel.setColor(App::ThemeProvider::getCurrentTheme()->backgroundColor);
        play_panel.setBorder(juce::Colours::black);
        play_panel.setJustify(juce::FlexBox::JustifyContent::center);
        play_panel.setAlign(juce::FlexBox::AlignContent::center);
        play_panel.setMargin(8);
        play_panel.setPadding(8);
        play_panel.addElements(std::vector<juce::Component *>({&play_button, &stop_button}));
        play_button.setButtonText(">");
        play_button.onClick = [this]
        { play(); };
        stop_button.setButtonText("||");
        stop_button.onClick = [this]
        {
            stop();
        };
        addAndMakeVisible(play_panel);

        //   toolbar.addElements(std::vector<juce::Component *>({&file_button, &play_button, &stop_button}));
        // addAndMakeVisible(toolbar);
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

    void save()
    {
        filechooser.reset(new juce::FileChooser("Save...", juce::File::getCurrentWorkingDirectory(),
                                                "*", true));

        filechooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser &fc) mutable
                                 {
                                              if (fc.getURLResults().size() > 0)
                                              {
                                                  auto u = fc.getURLResult();
                                                  auto name = u.getLocalFile().getFullPathName();
                                        //          setFile (std::move (u), name);
                                              }

                                              filechooser = nullptr; }, nullptr);
    }
    void new_graph()
    {
    }
    void open()
    {
    }
    void export_graph()
    {
    }

    void resized() override
    {
        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::column;

        fb.items.add(juce::FlexItem(getWidth(), 25, toolbar));
        fb.items.add(juce::FlexItem(getWidth(), 50, play_panel).withMargin(0));
        fb.items.add(juce::FlexItem(getWidth(), 50, stretcher).withFlex(1));

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
    MenuButton file_button;
    FlexWithColor toolbar;
    FlexWithColor play_panel;
    std::unique_ptr<juce::FileChooser> filechooser = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
