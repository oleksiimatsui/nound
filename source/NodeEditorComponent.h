#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"

class NodeComponent : public juce::Component {
public:

    class PinComponent : public juce::Component {
    public:
        PinComponent() {
                   theme = ThemeProvider::getCurrentTheme();
            setSize(theme->pinRadius, theme->pinRadius); // Set the size of the child component
        }
        void paint(juce::Graphics& g)  override {
            juce::Path p;
            p.addEllipse(0, 0, theme->pinRadius,theme->pinRadius);
            g.setColour(theme->pinColor);
            g.fillPath(p);
        }
        void resized() {
            setSize(theme->pinRadius,theme->pinRadius);
        }
    private:
    Theme * theme;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
    };

    bool selected = false;
    juce::Point<int> position;
    std::unordered_map<int, std::unique_ptr<PinComponent>> inputs;
    std::vector<PinComponent*> ordered_inputs;
    std::unordered_map<int, std::unique_ptr<PinComponent>> outputs;
    std::vector<PinComponent*> ordered_outputs;


    NodeComponent(juce::Point<int> _position)
    {
        theme = ThemeProvider::getCurrentTheme();
        auto pin = new PinComponent();
        ordered_inputs.push_back(pin);
        for (auto& p : ordered_inputs) {
            addAndMakeVisible(p);
        }
        position = _position;
        setTransform(juce::AffineTransform::translation(_position));
        setSize(100, 100);
    }

    ~NodeComponent() override
    {
        for (auto& n : ordered_inputs) delete n;
        inputs.clear();
    }

    void paint(juce::Graphics& g) {
        
        g.setColour((theme->nodeColor));
        g.fillRoundedRectangle(getX(), getY(),getWidth(),getHeight(), theme->nodeRounding);

        g.setColour((theme->nodeTextColor));
        g.drawText(juce::String(std::to_string(position.x) + " " + std::to_string(position.y)), getLocalBounds(), juce::Justification::centred, true);
    }

    void resized() override {
        for (auto& p : ordered_inputs) {
            p->setBounds(0, 0, 100, 300);
        }
    }

private:
Theme * theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeComponent);
};


class Wrapper : public juce::Component {

};

class NodeEditorComponent  : public juce::Component
{
public:

    struct NodeListener : public MouseListener {
        NodeEditorComponent* view;
        explicit NodeListener(NodeEditorComponent* component) : view(component) {}
        void mouseDrag(const juce::MouseEvent& e) override {
            auto node = (NodeComponent*)(e.originalComponent);
            view->nodeMouseDrag(node, e);
        }
        void mouseUp(const juce::MouseEvent& e) override {
            auto node = (NodeComponent*)(e.originalComponent);
            view->nodeMouseUp(node, e);
        }
        void mouseDown(const juce::MouseEvent& e) override {
            auto node = (NodeComponent*)(e.originalComponent);
            view->nodeMouseDown(node, e);
        }
    };


    std::unique_ptr<NodeListener> mouseListener;

    NodeEditorComponent()
    {
        ThemeProvider::setDefault();
        theme = ThemeProvider::getCurrentTheme();
        mouseListener = std::make_unique<NodeListener>(this);

        auto node = new NodeComponent(juce::Point<int>(22,22));
        node_components[0] = node;
   

       // wrapper.addNodes(node_components);

        for (auto& [_, n] : node_components) {
            n->addMouseListener(mouseListener.get(), true);
            addAndMakeVisible(n);
        }
    }

    ~NodeEditorComponent() override {
        for (auto& [_, n] : node_components) delete n;
        node_components.clear();
    }

    void mouseWheelMove(const juce::MouseEvent& mouseEvent, const juce::MouseWheelDetails& wheel) override {
        float amount = wheel.deltaY;
        auto mousepos = mouseEvent.getPosition();
        int direction = ((amount > 0) - (amount < 0));
        int from = scale;
        scale = scale + direction * 5;
        if (scale <= 5) scale = 5;
        repaint();
        scaleChange(mousepos, from);
    }

    void scaleChange(juce::Point<int> point, int from) {
        float coeff = float(scale) / float(from);
        for (auto& [_, n] : node_components) {
            n->setTransform(juce::AffineTransform()
                .scaled((float)from/100.0, (float)from/100.0)
                .translated(n->position)
                .scaled(coeff, coeff, point.x, point.y));

            auto p = getLocalPoint(n, juce::Point<int>(0, 0));
            n->position = p;
            n->repaint();
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        bool middleMouseDown =  e.mods.isMiddleButtonDown();
        if (middleMouseDown) {
            auto offset = e.getOffsetFromDragStart();
            for (auto& [_, n] : node_components) {
               //n->setTransform(n->scale.followedBy(n->translation.translated(offset)));
                n->setTransform(getScaleTranform().followedBy(juce::AffineTransform::translation(n->position).translated(offset)));
            }
        }
    }
    void mouseUp(const juce::MouseEvent& e) override {
        for (auto& [_, n] : node_components) {
            auto p = getLocalPoint(n, juce::Point<int>(0, 0));
            n->position = p;
        }
    }

    void nodeMouseDrag(NodeComponent* node, const juce::MouseEvent& mouseEvent) {
        auto offset = mouseEvent.getOffsetFromDragStart();
        node->setTransform(getScaleTranform().followedBy(juce::AffineTransform::translation(node->position).translated(offset * float(scale) / 100.0)));
    }
    void nodeMouseDown(NodeComponent* node, const juce::MouseEvent& mouseEvent) {
        node->selected = true;
        node->repaint();
    }
    void nodeMouseUp(NodeComponent* node, const juce::MouseEvent& mouseEvent) {
        auto p = getLocalPoint(node, juce::Point<int>(0, 0));
        node->position = p;
        node->selected = false;
        node->repaint();
    }


    void paint (juce::Graphics& g) override
    {

        g.fillAll((theme->editorColor)); 
        g.setColour((theme->nodeTextColor));
        g.drawText(juce::String(std::to_string(scale) + "%"), getLocalBounds(), juce::Justification::bottomLeft, true);

    }

    void resized() override
    {
        auto w = getWidth();
        auto h = getHeight();
    }

    void addNode() {

    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeEditorComponent);
    std::unordered_map<int, NodeComponent*> node_components;
    NodeComponent* node;
    NodeComponent* node2;
    Wrapper wrapper;
    Theme * theme;
    int scale = 100;
    float size = 10000;

    juce::AffineTransform getScaleTranform() {
        return juce::AffineTransform::scale(float(scale)/100.0);
    }
};

