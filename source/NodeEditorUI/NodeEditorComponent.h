#pragma once
#include <JuceHeader.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "Theme.h"
#include "NodeGraph.h"
#include "NodeTypes.h"
#include "NodeComponent.h"
#include "PinComponent.h"
#include "ConnectionComponent.h"

class NodeEditorComponent : public juce::Component, public GraphListener
{
public:
    void message(std::string text) override
    {
        DBG(text);
    };
    void NodeAdded(Node *node) override
    {
        auto n = new NodeComponent(juce::Point<int>(10, 10), (EditorNode *)node);
        node_components[node->id] = n;
        n->addMouseListener(mouseListener.get(), true);
        addAndMakeVisible(n);
    };
    void NodeDeleted(int id) override
    {
        auto c = node_components[id];
        c->removeMouseListener(mouseListener.get());
        removeChildComponent(c);
        node_components.erase(id);
    };
    void ConnectionAdded(Connection *c) override
    {
        PinComponent *pin1 = node_components[c->getNodeFromId()]->outputs[c->getPinFromNumber()];
        PinComponent *pin2 = node_components[c->getNodeToId()]->inputs[c->getPinToNumber()];
        ConnectionComponent *con = new ConnectionComponent(pin1, pin2);
        auto start = getLocalPoint(pin1, juce::Point<int>(0, 0)) + juce::Point<int>(theme->pinDiameter / 2, theme->pinDiameter / 2);
        auto end = getLocalPoint(pin2, juce::Point<int>(0, 0)) + juce::Point<int>(theme->pinDiameter / 2, theme->pinDiameter / 2);
        con->calculateBounds(start, end);
        connection_components[c->id] = con;
        con->addMouseListener(mouseListener.get(), false);
        addAndMakeVisible(con);
        con->toBack();
    };
    void ConnectionDeleted(int con_id) override
    {
        auto c = connection_components[con_id];
        c->removeMouseListener(mouseListener.get());
        removeChildComponent(c);
        connection_components.erase(con_id);
    }

    struct NodeListener : public MouseListener
    {
        NodeEditorComponent *view;
        explicit NodeListener(NodeEditorComponent *component) : view(component) {}
        void mouseDrag(const juce::MouseEvent &e) override
        {
            if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent))
            {
                view->nodeMouseDrag(node, e);
            }
            else if (auto pin = dynamic_cast<PinComponent *>(e.originalComponent))
            {
                view->pinMouseDrag(pin, e);
            }
        }
        void mouseUp(const juce::MouseEvent &e) override
        {
            if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent))
            {
                view->nodeMouseUp(node, e);
            }
            else if (auto pin = dynamic_cast<PinComponent *>(e.originalComponent))
            {
                view->pinMouseUp(pin, e);
            }
            // else if (auto edge = dynamic_cast<EdgeComponent *>(e.originalComponent)) {
            //  view->edgeMouseUp(edge, e);
            //}
        }
        void mouseDown(const juce::MouseEvent &e) override
        {
            auto node = (NodeComponent *)(e.originalComponent);
            if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent))
            {
                view->nodeMouseDown(node, e);
            }
            else if (auto connection = dynamic_cast<ConnectionComponent *>(e.originalComponent))
            {
                view->connectionMouseDown(connection, e);
            }
        }
        void mouseEnter(const juce::MouseEvent &e) override
        {
            if (auto pin = dynamic_cast<PinComponent *>(e.originalComponent))
            {
                view->pinMouseEnter(pin, e);
            }
        }
    };

    std::unique_ptr<NodeListener> mouseListener;

    NodeEditorComponent(Graph *g)
    {
        g->registerListener(this);
        setWantsKeyboardFocus(true);
        ThemeProvider::setDefault();
        theme = ThemeProvider::getCurrentTheme();
        mouseListener = std::make_unique<NodeListener>(this);
        connection_preview = std::make_unique<ConnectionPreview>();

        graph = g;

        int i = 0;
        for (auto &[id, n] : graph->getNodes())
        {
            node_components[id] = new NodeComponent(juce::Point<int>(i * 100 + 10, 10), (EditorNode *)n);
            i++;
        };

        for (auto &[id, c] : graph->getConnections())
        {
            PinComponent *pin1 = node_components[c->getNodeFromId()]->outputs[c->getPinFromNumber()];
            PinComponent *pin2 = node_components[c->getNodeToId()]->outputs[c->getPinToNumber()];
            connection_components[id] = new ConnectionComponent(pin1, pin2);
        };

        for (auto &[_, n] : node_components)
        {
            n->addMouseListener(mouseListener.get(), true);
            addAndMakeVisible(n);
        };
        for (auto &[_, n] : connection_components)
        {
            n->addMouseListener(mouseListener.get(), true);
            addAndMakeVisible(n);
        };

        refreshConnections();
    }

    ~NodeEditorComponent() override
    {
        for (auto &[_, n] : node_components)
            delete n;
        node_components.clear();
        delete graph;
    }

    void mouseWheelMove(const juce::MouseEvent &mouseEvent, const juce::MouseWheelDetails &wheel) override
    {
        float amount = wheel.deltaY;
        auto mousepos = mouseEvent.getPosition();
        int direction = ((amount > 0) - (amount < 0));
        int from = scale;
        scale = scale + direction * 5;
        if (scale <= 5)
            scale = 5;
        repaint();
        scaleChange(mousepos, from);
    }

    void scaleChange(juce::Point<int> point, int from)
    {
        float coeff = float(scale) / float(from);
        for (auto &[_, n] : node_components)
        {
            n->setTransform(juce::AffineTransform()
                                .scaled((float)from / 100.0, (float)from / 100.0)
                                .translated(n->position)
                                .scaled(coeff, coeff, point.x, point.y));

            auto p = getLocalPoint(n, juce::Point<int>(0, 0));
            n->position = p;
            n->repaint();
        }
        refreshConnections();
    };

    void mouseDrag(const juce::MouseEvent &e) override
    {
        bool middleMouseDown = e.mods.isMiddleButtonDown();
        if (middleMouseDown)
        {
            auto offset = e.getOffsetFromDragStart();
            for (auto &[_, n] : node_components)
            {
                n->setTransform(getScaleTranform().followedBy(juce::AffineTransform::translation(n->position).translated(offset)));
            }
        }
        refreshConnections();
    };
    void mouseUp(const juce::MouseEvent &e) override
    {
        for (auto &[_, n] : node_components)
        {
            auto p = getLocalPoint(n, juce::Point<int>(0, 0));
            n->position = p;
        }
    };
    bool keyPressed(const juce::KeyPress &key) override
    {
        auto code = key.getKeyCode();
        auto commandDown = key.getModifiers().isCommandDown();

        // deleting by delete, backspace or x;
        if (code == juce::KeyPress::deleteKey ||
            code == juce::KeyPress::backspaceKey ||
            code == 88)
        {
            deleteSelected();
        }
        return true;
    };
    void deleteSelected()
    {
        std::vector<int> ids = getSelectedNodes();
        for (auto &id : ids)
        {
            graph->deleteNode(id);
        }
        ids = getSelectedConnections();
        for (auto &id : ids)
        {
            graph->deleteConnection(id);
        }
    };

    std::vector<int> getSelectedNodes()
    {
        std::vector<int> ids;
        for (auto &[id, n] : node_components)
        {
            if (n->selected)
            {
                ids.push_back(id);
            }
        };
        return ids;
    };
    std::vector<int> getSelectedConnections()
    {
        std::vector<int> ids;
        for (auto &[id, n] : connection_components)
        {
            if (n->selected)
            {
                ids.push_back(id);
            }
        };
        return ids;
    };

    void mouseDown(const juce::MouseEvent &e) override
    {
        removeSelects();
    };
    void removeSelects()
    {
        for (auto &[_, n] : node_components)
        {
            n->selected = false;
            n->repaint();
        }
        for (auto &[_, c] : connection_components)
        {
            c->selected = false;
            c->repaint();
        }
    }

    void nodeMouseDrag(NodeComponent *node, const juce::MouseEvent &mouseEvent)
    {
        auto offset = mouseEvent.getOffsetFromDragStart();
        node->setTransform(getScaleTranform().followedBy(juce::AffineTransform::translation(node->position).translated(offset * float(scale) / 100.0)));
        refreshConnections();
    }
    void nodeMouseDown(NodeComponent *node, const juce::MouseEvent &mouseEvent)
    {
        removeSelects();
        node->toFront(false);
        node->selected = true;
        node->repaint();
    }
    void nodeMouseUp(NodeComponent *node, const juce::MouseEvent &mouseEvent)
    {
        auto p = getLocalPoint(node, juce::Point<int>(0, 0));
        node->position = p;
        node->repaint();
    }
    void connectionMouseDown(ConnectionComponent *connection, const juce::MouseEvent &mouseEvent)
    {
        removeSelects();
        connection->selected = true;
        connection->repaint();
    }

    void refreshConnections()
    {
        for (auto &[_, e] : connection_components)
        {
            auto start = getLocalPoint(e->pin_from, juce::Point<int>(0, 0)) + juce::Point<int>(theme->pinDiameter / 2, theme->pinDiameter / 2);
            auto end = getLocalPoint(e->pin_to, juce::Point<int>(0, 0)) + juce::Point<int>(theme->pinDiameter / 2, theme->pinDiameter / 2);
            e->calculateBounds(start, end);
            e->repaint();
            e->toBack();
        }
    }

    void pinMouseUp(PinComponent *pin, const juce::MouseEvent &mouseEvent)
    {
        juce::ignoreUnused(pin);
        auto relativeEvent = mouseEvent.getEventRelativeTo(this);
        auto position = relativeEvent.getPosition();
        connection_preview->currentEndPosition = position;
        removeChildComponent(connection_preview.get());
    }

    void pinMouseDrag(PinComponent *pin, const juce::MouseEvent &e)
    {
        auto relativeEvent = e.getEventRelativeTo(this);
        auto position = relativeEvent.getPosition();
        connection_preview->startPin = pin;
        connection_preview->currentEndPosition = position;
        connection_preview->calculateBounds(getLocalPoint(pin, juce::Point<int>(theme->pinDiameter / 2, theme->pinDiameter / 2)), position);
        addAndMakeVisible(connection_preview.get());
    }

    void pinMouseEnter(PinComponent *pin, const juce::MouseEvent &e)
    {
        auto relativeEvent = e.getEventRelativeTo(this);
        auto position = relativeEvent.getPosition();
        if (connection_preview->currentEndPosition == position)
        {
            addConnection(connection_preview->startPin, pin);
        }
    };

    void addConnection(PinComponent *pin1, PinComponent *pin2)
    {
        Connection *connection;
        try
        {
            connection = graph->addConnection(pin2->pin, pin1->pin);
        }
        catch (const std::invalid_argument &e)
        {
            return;
        }
    };

    void paint(juce::Graphics &g) override
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

    void addNode()
    {
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeEditorComponent);
    std::unordered_map<int, NodeComponent *> node_components;
    std::unordered_map<int, ConnectionComponent *> connection_components;
    NodeComponent *node;
    NodeComponent *node2;
    Theme *theme;
    int scale = 100;
    float size = 10000;
    Graph *graph;

    std::unique_ptr<ConnectionPreview> connection_preview;

    juce::AffineTransform getScaleTranform()
    {
        return juce::AffineTransform::scale(float(scale) / 100.0);
    }
};
