#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "NodeTypes.h"
#include "NodeComponent.h"
#include "PinComponent.h"
#include "ConnectionComponent.h"

class NodeEditorComponent  : public juce::Component
{
public:
    struct NodeListener : public MouseListener {
        NodeEditorComponent* view;
        explicit NodeListener(NodeEditorComponent* component) : view(component) {}
        void mouseDrag(const juce::MouseEvent& e) override {
            if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent)) {
                view->nodeMouseDrag(node, e);
            } else if (auto pin = dynamic_cast<PinComponent *>(e.originalComponent)) {
                view->pinMouseDrag(pin, e);
            }
        }
        void mouseUp(const juce::MouseEvent& e) override {
            if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent)) {
                view->nodeMouseUp(node, e);
            } else if (auto pin = dynamic_cast<PinComponent *>(e.originalComponent)) {
                view->pinMouseUp(pin, e);
            }
            //else if (auto edge = dynamic_cast<EdgeComponent *>(e.originalComponent)) {
               // view->edgeMouseUp(edge, e);
            //}
        }
        void mouseDown(const juce::MouseEvent& e) override {
            auto node = (NodeComponent*)(e.originalComponent);
            if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent)) {
                view->nodeMouseDown(node, e);
            } else if (auto pin = dynamic_cast<PinComponent *>(e.originalComponent)) {
              //view->pinMouseDown(pin, e);
            }
        }
    };

    std::unique_ptr<NodeListener> mouseListener;

    NodeEditorComponent()
    {
        ThemeProvider::setDefault();
        theme = ThemeProvider::getCurrentTheme();
        mouseListener = std::make_unique<NodeListener>(this);
        connection_preview = std::make_unique<ConnectionPreview>();

        graph = new Graph();

        graph->addNode(new StartNode());
        graph->addNode(new SpeakerNode());
        graph->addNode(new FileReader());

        int i=0;
        for (auto& n : graph->getNodes()) {
            node_components[n->id] = new NodeComponent(juce::Point<int>(i*100+10,10), n);
            i++;
        }

        for (auto& [_, n] : node_components) {
            n->addMouseListener(mouseListener.get(), true);
            addAndMakeVisible(n);
        }
    }

    ~NodeEditorComponent() override {
        for (auto& [_, n] : node_components) delete n;
        node_components.clear();
        delete graph;
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

    void pinMouseUp(PinComponent* pin, const juce::MouseEvent& mouseEvent) {
        
    }
    void pinMouseDrag(PinComponent *pin, const juce::MouseEvent &e){
        auto relativeEvent = e.getEventRelativeTo(this);
        auto position = relativeEvent.getPosition();
        connection_preview->startPin = pin;
        connection_preview->endPin = nullptr;
        connection_preview->currentEndPosition = position;
        connection_preview->calculateBounds(getLocalPoint(pin, juce::Point<int>(theme->pinDiameter / 2, theme->pinDiameter / 2)), position);
        addAndMakeVisible(connection_preview.get());
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
    Theme * theme;
    int scale = 100;
    float size = 10000;
    Graph *  graph;

    std::unique_ptr<ConnectionPreview> connection_preview;

    juce::AffineTransform getScaleTranform() {
        return juce::AffineTransform::scale(float(scale)/100.0);
    }
};

