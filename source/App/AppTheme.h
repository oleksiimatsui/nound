#pragma once
#include <JuceHeader.h>
#include <string>

namespace App
{

    class Theme
    {
    public:
        Theme(){};
        juce::Colour backgroundColor;
        juce::Colour textColor;
        juce::Colour dropdownMouseoverBackgroundColor;
    };

    class DefaultTheme : public Theme
    {
    public:
        DefaultTheme()
        {
            backgroundColor = juce::Colour::fromRGB(85, 82, 88);
            textColor = juce::Colour::fromRGB(255, 255, 255);
            dropdownMouseoverBackgroundColor = juce::Colour::fromRGB(62, 62, 62);
        };

    private:
    };

    static class ThemeProvider
    {
    public:
        static void setDefault()
        {
            currentTheme.reset(new DefaultTheme());
        }
        static Theme *getCurrentTheme()
        {
            if (currentTheme.get() == nullptr)
            {
                currentTheme.reset(new DefaultTheme());
            }
            return currentTheme.get();
        }

    private:
        static std::unique_ptr<Theme> currentTheme;
    };

    std::unique_ptr<Theme> ThemeProvider::currentTheme = nullptr;
}
