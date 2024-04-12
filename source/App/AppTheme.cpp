
#include "AppTheme.h"

namespace App
{
    Theme::Theme(){};

    DefaultTheme::DefaultTheme()
    {
        backgroundColor = juce::Colour::fromRGB(85, 82, 88);
        textColor = juce::Colour::fromRGB(255, 255, 255);
        dropdownMouseoverBackgroundColor = juce::Colour::fromRGB(62, 62, 62);
    };

    void ThemeProvider::setDefault()
    {
        currentTheme.reset(new DefaultTheme());
    };

    Theme *ThemeProvider::getCurrentTheme()
    {
        if (currentTheme.get() == nullptr)
        {
            currentTheme.reset(new DefaultTheme());
        }
        return currentTheme.get();
    }

    std::unique_ptr<Theme> ThemeProvider::currentTheme = nullptr;
}
