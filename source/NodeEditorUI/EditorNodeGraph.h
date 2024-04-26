#pragma once
#include <JuceHeader.h>
#include "NodeGraph.h"
#include "EditorNode.h"

class EditorNodeGraph : public Graph
{
public:
    EditorNodeGraph(){

    };

    ~EditorNodeGraph() override
    {
    }

    std::unordered_map<int, Node *> getNodes()
    {
        return graph.getNodes();
    }
    std::unordered_map<int, Connection *> getConnections()
    {
        return graph.getConnections();
    }
    void addNode(Node *node)
    {
        graph.addNode(node);
    }
    Connection *addConnection(Pin *pin1, Pin *pin2)
    {
        return graph.addConnection(pin1, pin2);
    }
    std::vector<int> getConnectionsOfNode(int id)
    {
        return graph.getConnectionsOfNode(id);
    }
    std::vector<int> getInputConnectionsOfNode(int id)
    {
        return graph.getInputConnectionsOfNode(id);
    }
    void deleteConnection(int id)
    {
        return graph.deleteConnection(id);
    }
    void deleteNode(int id)
    {
        return graph.deleteNode(id);
    }

    void registerListener(GraphListener *listener)
    {
        return graph.registerListener(listener);
    }

    void triggerPin(Output *pin, Value &Value)
    {
        return graph.triggerPin(pin, Value);
    }

    std::vector<Input *> getInputsOfOutput(Output *pin)
    {
        return graph.getInputsOfOutput(pin);
    }

protected:
    Graph graph;
    std::map<Node, EditorNode> node_data;
};