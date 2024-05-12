#pragma once
#include "string"
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "NodeEditorComponent.h"
#include "NodeDropdown.h"
#include "NodeGraph.h"
#include "NodeTypesFactory.h"
#include <fstream>

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
            e->setSize(e->getWidth(), getHeight() - padding * 2);
            juce::FlexItem f = juce::FlexItem(e->getWidth(), 1, *e).withMargin(margin);
            fb.items.add(f);
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexWithColor);
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuButton)
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StretchComponent)
};

class MainComponent : public juce::Component
{
public:
    MainComponent() : g(new RecoverableNodeGraph()), node_editor(g.get()),
                      dropdown_panel(g.get()),
                      stretcher(false, &dropdown_panel, &node_editor, 0.1),
                      playing(false)
    {
        setTitle("Welcome to Nound");
        setDescription("Sound node editor");
        setFocusContainerType(FocusContainerType::focusContainer);
        addAndMakeVisible(stretcher);

        factory.reset(new NoundTypesFactory());

        file_button.setButtonText("File");
        file_button.onClick = [&]
        {
            juce::PopupMenu menu;
            menu.addItem("New", [this]
                         { new_graph(); });
            menu.addItem("Open", [this]
                         { open(); });
            menu.addItem("Save As", [this]
                         { save_as(); });
            menu.addItem("Export", [this]
                         { export_graph(); });
            menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(file_button));
        };
        toolbar.setColor(App::ThemeProvider::getCurrentTheme()->darkerColor);
        toolbar.addElements(std::vector<juce::Component *>({&file_button}));
        addAndMakeVisible(toolbar);

        juce::Image play_img = getImageFromAssets("play_white.png");
        start_button.setImages(true, true, true, play_img, 1.0f, juce::Colour(), play_img, 0.5f, juce::Colour(), play_img, 0.5f, juce::Colour());
        start_button.setSize(36, 36);
        start_button.onClick = [this]
        { play(); };

        juce::Image pause_img = getImageFromAssets("pause_white.png");
        pause_button.setImages(true, true, true, pause_img, 1.0f, juce::Colour(), pause_img, 0.5f, juce::Colour(), pause_img, 0.5f, juce::Colour());
        pause_button.setSize(36, 36);
        pause_button.onClick = [this]
        { if(playing){
            pause();
         } else {
            resume();
         } };

        juce::Image stop_img = getImageFromAssets("stop_white.png");
        stop_button.setImages(true, true, true, stop_img, 1.0f, juce::Colour(), stop_img, 0.5f, juce::Colour(), stop_img, 0.5f, juce::Colour());
        stop_button.setSize(36, 20);
        stop_button.onClick = [this]
        {
            stop();
        };

        position_slider.setRange(0, 1);
        // position_slider.addListener(this);
        position_slider.setSize(200, 20);
        position_slider.setValue(0);
        position_slider.setSliderStyle(juce::Slider::LinearBar);
        position_slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, position_slider.getTextBoxWidth(), position_slider.getTextBoxHeight());

        position_slider.onValueChange = [this]()
        {
            //  if(position_slider.valueChanged)
            bool played = playing;
            if (playing)
            {
                pause();
            }
            player.setPosition(position_slider.getValue() * player.getLength());
            if (played)
                resume();
        };

        label.setText("position", juce::NotificationType::dontSendNotification);
        label.setSize(200, 20);
        v.addComponent(&label);
        v.setSize(200, 36);
        v.addComponent(&position_slider);

        play_panel.setColor(App::ThemeProvider::getCurrentTheme()->backgroundColor.darker(0));
        //    play_panel.setColor(juce::Colours::aliceblue);
        play_panel.setBorder(juce::Colours::black);
        play_panel.setJustify(juce::FlexBox::JustifyContent::center);
        play_panel.setAlign(juce::FlexBox::AlignContent::center);
        play_panel.setMargin(2);
        play_panel.setPadding(16);
        play_panel.addElements(std::vector<juce::Component *>({
            &start_button,
            &pause_button,
            &stop_button,
            &v,
        }));

        addAndMakeVisible(play_panel);
        setSize(500, 500);
    }
    ~MainComponent() override
    {
    }

    void paint(juce::Graphics &g)
    {
    }
    void build()
    {
        auto graph = g.get();
        Value d = nullptr;
        for (auto &[id, n] : graph->getNodes())
        {
            if (graph->getInputConnectionsOfNode(id).size() == 0)
            {
                n->trigger(d, nullptr);
            }
        };
    }
    void play()
    {
        build();
        auto graph = g.get();
        for (auto &[id, n] : graph->getNodes())
        {
            if (auto out = dynamic_cast<OutputNode *>(n))
            {
                if (out->result == nullptr)
                    return;
                player.setSource(out->result);
                player.prepareToPlay(SAMPLES_PER_BLOCK_EXPECTED, SAMPLE_RATE);
                player.setPosition(position_slider.getValue() * player.getLengthInSeconds() * SAMPLE_RATE);
                playing = true;
                player.Start();
            }
        };
    }
    void stop()
    {
        player.Stop();
        playing = false;
        player.setPosition(position_slider.getValue() * player.getLength());
    }

    void pause()
    {
        playing = false;
        player.Stop();
    }
    void resume()
    {
        playing = true;
        player.Start();
    }

    void save_as()
    {
        auto fileToSave = juce::File::createTempFile("saveChooserDemo");

        if (fileToSave.createDirectory().wasOk())
        {
            fileToSave = fileToSave.getChildFile("untitled.txt");
            fileToSave.deleteFile();
            juce::FileOutputStream outStream(fileToSave);
        }

        fc.reset(new juce::FileChooser("Save as",
                                       juce::File::getCurrentWorkingDirectory().getChildFile(fileToSave.getFileName()),
                                       "*.nound", true));

        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, fileToSave](const juce::FileChooser &chooser)
                        {
                            auto result = chooser.getURLResult();
                            auto name = result.isEmpty() ? juce::String()
                                                         : (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                                 : result.toString(true));

                            selected_file_path = name;
                            saveGraphInfo(name);
                        });
    }
    void new_graph()
    {
        GraphInfo info;
        g.get()->recover(info, factory.get());
        node_editor.update();
    }
    void open()
    {
        fc.reset(new juce::FileChooser("Open", juce::File::getCurrentWorkingDirectory(),
                                       "*.nound", true));

        fc->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this](const juce::FileChooser &chooser)
                        {
                            auto result = chooser.getURLResult();
                            auto name = result.isEmpty() ? juce::String()
                                                         : (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                                 : result.toString(true));

                            selected_file_path = name;
                            setGraphInfo(name);
                        });
    }
    void export_graph()
    {
        auto fileToSave = juce::File::createTempFile("audio.wav");

        if (fileToSave.createDirectory().wasOk())
        {
            fileToSave = fileToSave.getChildFile("audio.wav");
            fileToSave.deleteFile();
            juce::FileOutputStream outStream(fileToSave);
        }

        fc.reset(new juce::FileChooser("Export",
                                       juce::File::getCurrentWorkingDirectory().getChildFile(fileToSave.getFileName()),
                                       "*.wav", true));

        fc->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, fileToSave](const juce::FileChooser &chooser)
                        {
                            auto result = chooser.getURLResult();
                            auto name = result.isEmpty() ? juce::String()
                                                         : (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                                 : result.toString(true));

                            selected_file_path = name;
                            exportToFile(name.toStdString());
                        });
    }

    void resized() override
    {
        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::column;
        // play_button.changeWidthToFitText();
        // stop_button.changeWidthToFitText();
        file_button.changeWidthToFitText();
        fb.items.add(juce::FlexItem(getWidth(), 25, toolbar));
        fb.items.add(juce::FlexItem(getWidth(), 50, play_panel).withMargin(0));
        fb.items.add(juce::FlexItem(getWidth(), 50, stretcher).withFlex(1));
        fb.performLayout(getLocalBounds());
    }

    void setGraphInfo(juce::String path)
    {
        stop();
        std::ifstream file(path.getCharPointer());
        if (file.is_open())
        {
            GraphInfo info;
            file >> info;
            file.close();
            g.get()->recover(info, factory.get());
            node_editor.update();
        }
        else
        {
            std::cerr << "Error: Unable to open file." << std::endl;
        }
    }
    void saveGraphInfo(juce::String path)
    {
        std::unordered_map<int, juce::Point<int>> positions;
        for (auto &[id, node] : g.get()->getNodes())
        {
            positions[id] = node_editor.getNodePosition(id);
        }
        auto info = g.get()->get_info(positions);
        std::ofstream file(path.getCharPointer(), std::ofstream::out | std::ofstream::trunc);

        if (file.is_open())
        {
            file << info;
            file.close();
            std::cout << "GraphInfo object saved to file." << std::endl;
        }
        else
        {
            std::cerr << "Error: Unable to open file." << std::endl;
        }
    }

    juce::Image getImageFromAssets(std::string str)
    {
        juce::File imageFile = juce::File::getCurrentWorkingDirectory().getParentDirectory().getChildFile("assets/" + str);
        juce::Image image = juce::ImageFileFormat::loadFrom(imageFile);
        return image;
    }

    void exportToFile(std::string path)
    {
        stop();
        juce::File file = juce::File(path);
        if (file.existsAsFile())
            file.deleteFile();
        juce::AudioBuffer<float> buffer;
        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer;
        build();
        PositionableSource *output = nullptr;
        for (auto &[id, n] : g.get()->getNodes())
        {
            if (auto out = dynamic_cast<OutputNode *>(n))
            {
                output = (out->result);
            }
        };
        if (output == nullptr)
            return;
        output->setPosition(0);
        const int size = 480;
        writer.reset(format.createWriterFor(new juce::FileOutputStream(file),
                                            SAMPLE_RATE,
                                            2,
                                            24,
                                            {},
                                            0));
        output->prepareToPlay(480, SAMPLE_RATE);
        output->setPosition();
        while (output->isPlaying())
        {
            juce::AudioBuffer<float> buffer;
            buffer.setSize(2, 480);
            output->getNextAudioBlock(juce::AudioSourceChannelInfo(&buffer, 0, size));

            if (writer != nullptr)
                writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        }
        output->releaseResources();
    }

private:
    Player player;
    std::unique_ptr<RecoverableNodeGraph> g;
    std::unique_ptr<TypesRecoverFactory> factory;
    NodeEditorComponent node_editor;
    DropdownComponent dropdown_panel;
    StretchComponent stretcher;
    juce::ImageButton start_button;
    juce::ImageButton pause_button;
    juce::ImageButton stop_button;
    MenuButton file_button;
    FlexWithColor toolbar;
    FlexWithColor play_panel;
    juce::Slider position_slider;
    juce::Label label;
    Vertical v;
    std::unique_ptr<juce::FileChooser> fc = nullptr;
    juce::String selected_file_path;
    bool playing;
    const int SAMPLE_RATE = 4800;
    const int SAMPLES_PER_BLOCK_EXPECTED = 480;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

class MainComponentTest : public juce::Component
{
public:
    MainComponentTest()
    {
        setTitle("Welcome to Nound");
        setDescription("Sound node editor");
        setFocusContainerType(FocusContainerType::focusContainer);
        setSize(500, 500);
    }
};
