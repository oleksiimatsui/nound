#pragma once
#include <JuceHeader.h>
#include "Theme.h"
class FileInputListener
{
public:
    virtual void setFile(juce::URL *resource) = 0;
};
class FileInput : public juce::Component
{
public:
    FileInput(FileInputListener *l)
    {
        listener = l;
        button.onClick = [this]()
        { browseButtonClicked(); };
        button.setColour(juce::Label::textColourId, ThemeProvider::getCurrentTheme()->nodeTextColor);
        juce::Font f;
        f.setHeight(ThemeProvider::getCurrentTheme()->nodeTextHeight);
        label.setColour(juce::Label::textColourId, ThemeProvider::getCurrentTheme()->nodeTextColor);
        label.setFont(f);
        addAndMakeVisible(button);
        label.setSize(0, ThemeProvider::getCurrentTheme()->nodeTextHeight);
        button.setSize(0, ThemeProvider::getCurrentTheme()->nodeTextHeight + 4);

        addAndMakeVisible(label);

        setSize(0, 0);

        alignElements();
        int h = label.getBottom();
        setSize(getWidth(), h);
    };
    juce::FlexItem getFlexItem(Component &comp, int width)
    {
        return juce::FlexItem(width, comp.getHeight(), comp);
    };
    void alignElements()
    {

        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::column;

        fb.items.addArray({
            getFlexItem(button, getWidth()),
            getFlexItem(label, getWidth()),
        });

        fb.performLayout(getLocalBounds());
    }
    ~FileInput(){

    };
    void paint(juce::Graphics &g) {

    };
    void resized() override
    {
        alignElements();
    };
    void browseButtonClicked()
    {
        filechooser.reset(new juce::FileChooser("Choose a file to open...", juce::File::getCurrentWorkingDirectory(),
                                                "*", true));

        filechooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser &fc) mutable
                                 {
                                              if (fc.getURLResults().size() > 0)
                                              {
                                                  auto u = fc.getURLResult();
                                                  auto name = u.getLocalFile().getFullPathName();
                                                  setFile (std::move (u), name);
                                              }

                                              filechooser = nullptr; }, nullptr);
    }
    void setFile(juce::URL resource, juce::String name)
    {
        currentAudioFile = std::move(resource);
        listener->setFile(&currentAudioFile);
        label.setText(name, juce::NotificationType::sendNotification);
    }

private:
    juce::TextButton button{"Browse"};
    juce::Label label{{}, "file is not chosen"};
    FileInputListener *listener;
    juce::URL currentAudioFile;
    std::unique_ptr<juce::FileChooser> filechooser = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileInput);
};