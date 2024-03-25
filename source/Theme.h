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
    unsigned int nodeRounding;
    unsigned int pinRadius;
    unsigned int pinSpacing;
};

class DefaultTheme : public Theme{
public:
    DefaultTheme(){
        editorColor = juce::Colour::fromRGB(39, 24, 36);
        nodeColor = juce::Colour::fromRGB(110, 110, 110);
        nodeHeaderColor = juce::Colour::fromRGB(47, 72, 119);
        nodeInputColor = juce::Colour::fromRGB(217, 217, 217);
        controlPinColor = juce::Colour::fromRGB(217, 217, 217);
        soundPinColor = juce::Colour::fromRGB(173, 255, 0);
        pinColor =juce::Colour::fromRGB(173, 255, 0);
        nodeTextColor = juce::Colour::fromRGB(255, 255, 255);
        pinRadius = 10;
        pinSpacing = 30;
        nodeRounding = 16;
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
