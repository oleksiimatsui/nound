#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "SettableComponent.h"
#include "ValueRef.h"

class Selector : public SettableComponent, public juce::ComboBox::Listener
{
public:
    Selector(IntRef *vr, juce::ComboBox::Listener *l, std::vector<std::string> vs, int first_element_index = 0) : SettableComponent(vr)
    {
        listener = l;
        c = new juce::ComboBox();
        for (int i = 0; i < vs.size(); i++)
        {
            c->addItem(vs[i], i + first_element_index);
        }
        c->setSelectedId(first_element_index);
        c->addListener(this);
        setComponent(c);
        setSize(1000, 20);
        addAndMakeVisible(c);
    }
    void comboBoxChanged(juce::ComboBox *c) override
    {
        int v = c->getSelectedId();
        ((IntRef *)(value_ref))->value = v;
        update();
        listener->comboBoxChanged(c);
    }
    void update() override
    {
        c->setSelectedId((int)(((IntRef *)(value_ref))->value));
    }

private:
    juce::ComboBox *c;
    juce::ComboBox::Listener *listener;
};

class FileInput : public SettableComponent
{
public:
    class Listener
    {
    public:
        virtual void fileChanged()
        {
        }
    };

    FileInput(StringRef *vr, FileInput::Listener *l) : SettableComponent(vr)
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
                                                  setFile (name);
                                              }

                                              filechooser = nullptr; }, nullptr);
    }
    void setFile(juce::String name)
    {
        StringRef *strref = (StringRef *)value_ref;
        strref->value = name.toStdString();
        label.setText(name, juce::NotificationType::dontSendNotification);
    }
    void update() override
    {
        label.setText(toString(), juce::NotificationType::dontSendNotification);
    }

private:
    juce::TextButton button{"Browse"};
    juce::Label label{{}, "file is not chosen"};
    Listener *listener;
    std::unique_ptr<juce::FileChooser> filechooser = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileInput);
};

class NumberInput : public SettableComponent, public juce::Slider::Listener
{
public:
    class Listener
    {
    public:
        virtual void valueChanged() {

        };
    };
    void sliderValueChanged(juce::Slider *slider) override
    {
        *value = slider->getValue();
        listener->valueChanged();
    }
    NumberInput(NumberInput::Listener *_node, float min, float max, FloatRef *val) : SettableComponent(val)
    {
        listener = _node;
        value = &(val->value);
        slider.setRange(min, max);
        slider.addListener(this);
        slider.setSliderStyle(juce::Slider::LinearBar);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, slider.getTextBoxWidth(), slider.getTextBoxHeight());
        auto theme = ThemeProvider::getCurrentTheme();
        slider.setSize(100, theme->nodeTextHeight * 1.5);
        slider.setValue((*value));
        addAndMakeVisible(slider);
        setSize(100, theme->nodeTextHeight * 1.5);
    };
    void resized() override
    {
        slider.setBounds(getLocalBounds());
    };
    void update() override
    {
        auto fn = [this]()
        {
            slider.setValue(*value);
        };
        if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        {
            fn();
        }
        else
        {
            juce::MessageManager::callAsync(std::move(fn));
        }
    }

private:
    float *value;
    juce::Slider slider;
    Listener *listener;
};

class Vertical : public juce::Component
{
public:
    Vertical(){

    };
    void addComponent(juce::Component *c)
    {
        components.push_back(c);
        addAndMakeVisible(c);
    }
    juce::FlexItem getFlexItem(Component *comp, int width)
    {
        return juce::FlexItem(width, comp->getHeight(), *comp);
    };
    void alignElements()
    {

        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::column;

        for (auto &component : components)
        {
            fb.items.add(getFlexItem(component, getWidth()));
        }

        fb.performLayout(getLocalBounds());
    }
    ~Vertical(){

    };
    void paint(juce::Graphics &g) {

    };
    void resized() override
    {
        alignElements();
    };
    void browseButtonClicked()
    {
    }

private:
    std::vector<juce::Component *> components;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Vertical);
};