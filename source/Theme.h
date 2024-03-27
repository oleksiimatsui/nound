#pragma once
#include <string>

class Theme {
public:
Theme(){};
    juce::Colour  editorColor;
    juce::Colour  nodeColor;
    juce::Colour  nodeTextColor;
    juce::Colour  nodeHeaderColor;
    juce::Colour  nodeText;
    juce::Colour  pinColor;
    juce::Colour  nodeInputColor;
    juce::Colour  controlPinColor;
    juce::Colour  soundPinColor;
    juce::Colour  connectionsColor;
    unsigned int nodeRounding;
    unsigned int pinDiameter;
    unsigned int pinSpacing;
    int nodeWidth;
    int connectionsThickness;
};

class DefaultTheme : public Theme{
public:
    DefaultTheme(){
        editorColor = juce::Colour::fromRGB(39, 24, 36);
        nodeColor = juce::Colour::fromRGBA(110, 110, 110, 0.72*255);
        nodeHeaderColor = juce::Colour::fromRGB(47, 72, 119);
        nodeInputColor = juce::Colour::fromRGB(217, 217, 217);
        controlPinColor = juce::Colour::fromRGB(217, 217, 217);
        soundPinColor = juce::Colour::fromRGB(224, 255, 159);
        pinColor =juce::Colour::fromRGB(173, 255, 0);
        nodeTextColor = juce::Colour::fromRGB(255, 255, 255);
        connectionsColor = juce::Colour::fromRGB(224, 255, 159);
        pinDiameter = 10;
        pinSpacing = 12;
        nodeRounding = 16;
        nodeWidth = 100;
        connectionsThickness = 4;
    };
private:
uint32_t hexToUInt32(const std::string& hex) {
    std::istringstream converter(hex);
    unsigned int value;
    converter >> std::hex >> value;
    return value;
}
};



static class ThemeProvider {
public:
    static void setDefault(){
        currentTheme.reset(new DefaultTheme());
    }
    static Theme* getCurrentTheme(){
        return currentTheme.get();
    }
private:
    static std::unique_ptr<Theme> currentTheme;
};

std::unique_ptr<Theme> ThemeProvider::currentTheme = nullptr;
