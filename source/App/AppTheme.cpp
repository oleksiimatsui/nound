
#include "AppTheme.h"

namespace App
{
    Theme::Theme(){};

    DefaultTheme::DefaultTheme()
    {
        backgroundColor = juce::Colour::fromRGB(92, 87, 93);
        textColor = juce::Colour::fromRGB(255, 255, 255);
        dropdownMouseoverBackgroundColor = juce::Colour::fromRGB(62, 62, 62);
        darkerColor = juce::Colour::fromRGBA(58, 59, 67, 0.72 * 255);
        hoveredColor = juce::Colour::fromRGBA(0, 0, 0, 0.5 * 255);
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
