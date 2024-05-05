#pragma once
#include <JuceHeader.h>
#include "NodeEditorComponent.h"
#include "NodeGraph.h"
#include <fstream>

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
        std::map<int, std::string> input_values;
        std::vector<std::string> internal_values;
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

// Serialize function for GraphInfo
std::ostream &operator<<(std::ostream &os, const GraphInfo &graph)
{
    // Serialize nodes
    os << graph.nodes.size() << "\n";
    for (const auto &pair : graph.nodes)
    {
        const auto &key = pair.first;
        os << key << " ";
        const auto &node = pair.second;
        os << node.type_id << " " << node.x << " " << node.y << " ";
        os << node.input_values.size() << " ";
        for (const auto &[id, value] : node.input_values)
        {
            os << id << " ";
            os << value << " ";
        }
        os << node.internal_values.size() << " ";
        for (const auto &value : node.internal_values)
        {
            os << value << " ";
        }
    }

    // Serialize connections
    os << graph.connections.size() << "\n";
    for (const auto &pair : graph.connections)
    {
        os << pair.first << " ";
        const auto &connection = pair.second;
        os << connection.node_from_id << " " << connection.node_to_id << " ";
        os << connection.pin_from_number << " " << connection.pin_to_number << "\n";
    }

    return os;
}

// Deserialize function for GraphInfo
std::istream &operator>>(std::istream &is, GraphInfo &graph)
{
    // Deserialize nodes
    int num_nodes;
    is >> num_nodes;
    is.ignore(); // Ignore the newline character after reading num_nodes
    for (int i = 0; i < num_nodes; ++i)
    {
        int key;
        is >> key;
        GraphInfo::node node;
        is >> node.type_id >> node.x >> node.y;

        // Deserialize input_values
        int num_input_values;
        is >> num_input_values;
        for (int j = 0; j < num_input_values; ++j)
        {
            int key;
            std::string value_str;
            is >> key >> value_str;
            node.input_values[key] = value_str;
        }

        // Deserialize internal_values
        int num_internal_values;
        is >> num_internal_values;
        for (int j = 0; j < num_internal_values; ++j)
        {
            std::string val;
            is >> val;
            node.internal_values.push_back(val);
        }

        graph.nodes[key] = node;
    }

    // Deserialize connections
    int num_connections;
    is >> num_connections;
    is.ignore(); // Ignore the newline character after reading num_connections
    for (int i = 0; i < num_connections; ++i)
    {
        int key;
        is >> key;
        GraphInfo::connection connection;
        is >> connection.node_from_id >> connection.node_to_id;
        is >> connection.pin_from_number >> connection.pin_to_number;
        graph.connections[key] = connection;
        is.ignore();
    }

    return is;
}

class RecoverableNodeGraph : public Graph
{
public:
    RecoverableNodeGraph() : Graph(){};
    RecoverableNodeGraph(GraphInfo info, TypesRecoverFactory *factory) : Graph()
    {
        recover(info, factory);
    };

    ~RecoverableNodeGraph() override
    {
    }
    void clear_graph()
    {
        for (auto &[_, n] : nodes)
            delete n;
        nodes.clear();
        for (auto &[_, n] : connections)
            delete n;
        connections.clear();
    }
    void recover(GraphInfo info, TypesRecoverFactory *factory)
    {
        clear_graph();
        for (auto &[id, info] : info.nodes)
        {
            auto node = factory->getNode(info.type_id);
            nodes[id] = node;
            node->id = id;
            node->graph = this;
            Graph::id = std::max(id, Graph::id);
            for (auto &[in_id, val] : info.input_values)
            {
                node->input_components[in_id]->fromString(val);
                node->input_components[in_id]->update();
            }
            for (int i = 0; i < info.internal_values.size(); i++)
            {
                auto &string = info.internal_values[i];
                node->internal_components[i]->fromString(string);
                node->internal_components[i]->update();
            }
            node->x = info.x;
            node->y = info.y;
        }
        for (auto &[id, info] : info.connections)
        {
            auto pin1 = nodes[info.node_from_id]->outputs[info.pin_from_number];
            auto pin2 = nodes[info.node_to_id]->inputs[info.pin_to_number];
            ConnectionBuilder factory;
            factory.addPin(pin1);
            factory.addPin(pin2);
            Connection *connection;
            connection = factory.build();
            connection->id = id;
            connections[id] = connection;
            Graph::id = std::max(id, Graph::id);
        }
    }

    GraphInfo get_info(std::map<int, juce::Point<int>> positions)
    {
        GraphInfo info;
        for (auto &[id, n] : nodes)
        {
            auto node = (EditorNode *)n;
            GraphInfo::node node_info;
            node_info.x = 0;
            node_info.y = 0;
            auto position = positions[id];
            node_info.x = position.x;
            node_info.y = position.y;
            node_info.type_id = node->type_id;
            for (auto &[pin_id, c] : node->input_components)
            {
                if (c != nullptr)
                    node_info.input_values[pin_id] = c->toString();
            }
            for (auto &internal : node->internal_components)
            {
                if (internal != nullptr)
                    node_info.internal_values.push_back(internal->toString());
            }
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