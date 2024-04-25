#pragma once
#include <JuceHeader.h>
#include <string>

namespace App
{

    class Theme
    {
    public:
        Theme();
        juce::Colour backgroundColor;
        juce::Colour textColor;
        juce::Colour dropdownMouseoverBackgroundColor;
        juce::Colour darkerColor;
        juce::Colour hoveredColor;
    };

    class DefaultTheme : public Theme
    {
    public:
        DefaultTheme();

    private:
    };

    static class ThemeProvider
    {
    public:
        static void setDefault();
        static Theme *getCurrentTheme();

    private:
        static std::unique_ptr<Theme> currentTheme;
    };
}
