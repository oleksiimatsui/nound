#pragma once
#include <JuceHeader.h>
#include <string>

class Theme
{
public:
    Theme(){};
    juce::Colour editorColor;
    juce::Colour nodeColor;
    juce::Colour nodeTextColor;
    juce::Colour nodeHeaderColor;
    juce::Colour nodeText;
    juce::Colour pinColor;
    juce::Colour nodeInputColor;
    juce::Colour controlPinColor;
    juce::Colour soundPinColor;
    juce::Colour connectionsColor;
    juce::Colour selectedConnectionColor;
    juce::Colour selectedNodeBorderColor;
    unsigned int nodeRounding;
    unsigned int pinDiameter;
    unsigned int pinSpacing;
    int nodeWidth;
    int connectionsThickness;
    int selectedNodeBorderWidth;
    int padding;
    int nodeTextHeight;
    int headerHeight;
};

class DefaultTheme : public Theme
{
public:
    DefaultTheme()
    {
        editorColor = juce::Colour::fromRGB(39, 24, 36);
        nodeColor = juce::Colour::fromRGBA(58, 59, 67, 0.72 * 255);
        nodeHeaderColor = juce::Colour::fromRGB(74, 47, 119);
        nodeInputColor = juce::Colour::fromRGB(217, 217, 217);
        controlPinColor = juce::Colour::fromRGB(109, 211, 255);
        soundPinColor = juce::Colour::fromRGB(217, 255, 138);
        pinColor = juce::Colour::fromRGB(173, 255, 0);
        nodeTextColor = juce::Colour::fromRGB(245, 245, 245);
        selectedConnectionColor = juce::Colour::fromRGB(255, 255, 255);
        selectedNodeBorderColor = juce::Colour::fromRGB(255, 255, 255);
        selectedNodeBorderWidth = 2;
        pinDiameter = 10;
        pinSpacing = 12;
        nodeRounding = 8;
        nodeWidth = 150;
        connectionsThickness = 4;
        padding = 12;
        nodeTextHeight = 12;
        headerHeight = 24;
    };

private:
    uint32_t hexToUInt32(const std::string &hex)
    {
        std::istringstream converter(hex);
        unsigned int value;
        converter >> std::hex >> value;
        return value;
    }
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
        return currentTheme.get();
    }

private:
    static std::unique_ptr<Theme> currentTheme;
};

std::unique_ptr<Theme> ThemeProvider::currentTheme = nullptr;
