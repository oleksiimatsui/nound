#pragma once
#include <JuceHeader.h>
#include "NodeGraph.h"
#include "EditorNode.h"

class TypesRecoverFactory
{
public:
    virtual EditorNode *getNode(int type_id) = 0;
};

struct GraphInfo
{
    struct node
    {
        int type_id;
        int x;
        int y;
        std::map<int, Value> input_values;
        std::vector<Value> internal_values;
    };
    struct connection
    {
        int node_from_id;
        int node_to_id;
        int pin_from_number;
        int pin_to_number;
    };
    std::map<int, node> nodes;
    std::unordered_map<int, connection> connections;
};

class RecoverableNodeGraph : public Graph
{
public:
    RecoverableNodeGraph(){

    };

    ~RecoverableNodeGraph() override
    {
    }

    void recover(GraphInfo info, TypesRecoverFactory &factory)
    {
        for (auto &[id, info] : info.nodes)
        {
            auto node = factory.getNode(info.type_id);
            nodes[id] = node;
            Graph::id = std::max(id, Graph::id);
            node->assignInputs(info.input_values);
            node->assignInternals(info.internal_values);
            node->x = info.x;
            node->y = info.y;
        }
        for (auto &[id, info] : info.connections)
        {
            auto pin1 = nodes[info.node_from_id]->outputs[info.pin_from_number];
            auto pin2 = nodes[info.node_to_id]->inputs[info.pin_to_number];
            Graph::addConnection(pin1, pin2);
        }
    }

    GraphInfo get_info(TypesRecoverFactory &factory, NodeEditorComponent *editor_component)
    {
        GraphInfo info;
        for (auto &[id, n] : nodes)
        {
            auto node = (EditorNode *)n;
            GraphInfo::node node_info;
            auto position = editor_component->getNodePosition(id);
            node_info.x = position.x;
            node_info.y = position.y;
            node_info.type_id = node->type_id;
            node_info.input_values = node->getInputs();
            node_info.internal_values = node->getInternals();
            info.nodes[id] = node_info;
        }
        for (auto &[id, c] : connections)
        {
            GraphInfo::connection con_info;
            con_info.node_from_id = c->getNodeFromId();
            con_info.node_to_id = c->getNodeToId();
            con_info.pin_from_number = c->getPinFromNumber();
            con_info.pin_to_number = c->getPinToNumber();
            info.connections[id] = con_info;
        }
        return info;
    }

    // EditorNode *getEditorNode(int node_id)
    // {
    //     return node_data[node_id];
    // }

    // std::unordered_map<int, Node *> getNodes()
    // {
    //     return graph.getNodes();
    // }
    // std::unordered_map<int, Connection *> getConnections()
    // {
    //     return graph.getConnections();
    // }
    // void addNode(EditorNode *node)
    // {
    //     graph.addNode(node);
    //     int id = node->id;
    //     node_data[id] = node;
    // }
    // Connection *addConnection(Pin *pin1, Pin *pin2)
    // {
    //     return graph.addConnection(pin1, pin2);
    // }
    // std::vector<int> getConnectionsOfNode(int id)
    // {
    //     return graph.getConnectionsOfNode(id);
    // }
    // std::vector<int> getInputConnectionsOfNode(int id)
    // {
    //     return graph.getInputConnectionsOfNode(id);
    // }
    // void deleteConnection(int id)
    // {
    //     return graph.deleteConnection(id);
    // }
    // void deleteNode(int id)
    // {
    //     return graph.deleteNode(id);
    // }
    // void registerListener(GraphListener *listener)
    // {
    //     return graph.registerListener(listener);
    // }
    // void triggerPin(Output *pin, Value &Value)
    // {
    //     return graph.triggerPin(pin, Value);
    // }
    // std::vector<Input *> getInputsOfOutput(Output *pin)
    // {
    //     return graph.getInputsOfOutput(pin);
    // }

protected:
};