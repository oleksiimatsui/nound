#include <string>

class Theme {
public:
Theme(){};
    unsigned int editorColor;
    unsigned int nodeColor;
    unsigned int nodeHeaderColor;
    unsigned int nodeText;
    unsigned int pinColor;
    unsigned int pinRadius;
    unsigned int pinSpacing;
};

class DefaultTheme : public Theme{
public:
    DefaultTheme(){
        editorColor = 0xFF242424;
        nodeColor = 0xCF4F4F4F;
        nodeHeaderColor = 0xCF656464;
        nodeText = 1;
        pinColor = 0xFF767676;
        pinRadius = 10;
        pinSpacing = 30;
};
};



static class ThemeProvider {
public:
    static void setDefault(){
        currentTheme = new DefaultTheme();
    }
    static Theme* getCurrentTheme(){
        return currentTheme;
    }
private:
    static Theme * currentTheme;
};

Theme* ThemeProvider::currentTheme = nullptr;
