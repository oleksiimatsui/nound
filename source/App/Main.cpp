
#include <JuceHeader.h>
#include "MainComponent.h"
#include "AppTheme.h"

class LookAndFeel : public juce::LookAndFeel_V4
{
    void drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::Colour bg = App::ThemeProvider::getCurrentTheme()->darkerColor;
        LookAndFeel_V4::drawButtonBackground(g, button, bg, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    }
    void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override
    {
        juce::Colour bg = App::ThemeProvider::getCurrentTheme()->darkerColor;
        g.setColour(bg);
        g.fillRect(0, 0, width, height);
    }
    void drawLabel(juce::Graphics &g, juce::Label &label) override
    {
        if (label.getFont().getTypefaceName() == "SliderLabelFont")
        {
            g.setColour(label.findColour(juce::Label::textColourId));
            g.setFont(label.getFont().withHeight(ThemeProvider::getCurrentTheme()->nodeTextHeight)); // Set your desired font size here
            g.drawFittedText(label.getText(), label.getLocalBounds(), label.getJustificationType(), 1);
        }
        else
        {
            LookAndFeel_V4::drawLabel(g, label);
        }
    }
};

class Nound final : public juce::JUCEApplication
{
public:
    Nound() {}

    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String &commandLine) override
    {
        juce::LookAndFeel::setDefaultLookAndFeel(&look_and_feel);
        //    look_and_feel.setColourScheme(juce::LookAndFeel_V4::getMidnightColourScheme());
        juce::ignoreUnused(commandLine);
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String &commandLine) override
    {
        juce::ignoreUnused(commandLine);
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name,
                             juce::Colours::aliceblue,
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setFullScreen(true);
            setVisible(true);
            App::ThemeProvider::setDefault();

            App::Theme *theme = App::ThemeProvider::getCurrentTheme();
            setBackgroundColour(theme->backgroundColor);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    LookAndFeel look_and_feel;
};

START_JUCE_APPLICATION(Nound)
