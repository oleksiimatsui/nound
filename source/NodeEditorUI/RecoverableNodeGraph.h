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

// Serialize function for GraphInfo
std::ostream &operator<<(std::ostream &os, const GraphInfo &graph)
{
    // Serialize nodes
    os << graph.nodes.size() << "\n";
    for (const auto &pair : graph.nodes)
    {
        const auto &node = pair.second;
        os << node.type_id << " " << node.x << " " << node.y << " ";
        os << node.input_values.size() << " ";
        for (const auto &input_pair : node.input_values)
        {
            os << input_pair.first << " ";

            // Serialize based on type
            const std::type_info &info = input_pair.second.type();
            if (info == typeid(int))
            {
                os << std::any_cast<int>(input_pair.second) << " ";
            }
            else if (info == typeid(double))
            {
                os << std::any_cast<double>(input_pair.second) << " ";
            }
            else if (info == typeid(std::string))
            {
                os << std::any_cast<std::string>(input_pair.second) << " ";
            } // Add more types as needed
        }
        os << node.internal_values.size() << " ";
        for (const auto &value : node.internal_values)
        {
            // Serialize based on type
            const std::type_info &info = value.type();
            if (info == typeid(int))
            {
                os << std::any_cast<int>(value) << " ";
            }
            else if (info == typeid(double))
            {
                os << std::any_cast<double>(value) << " ";
            }
            else if (info == typeid(std::string))
            {
                os << std::any_cast<std::string>(value) << " ";
            } // Add more types as needed
        }
    }

    // Serialize connections
    os << graph.connections.size() << "\n";
    for (const auto &pair : graph.connections)
    {
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

            if (value_str == "null")
            {
                // Handle null value
                node.input_values[key] = std::any();
            }
            else
            {
                // Deserialize based on type
                if (value_str == "int")
                {
                    int intValue;
                    is >> intValue;
                    node.input_values[key] = intValue;
                }
                else if (value_str == "double")
                {
                    double doubleValue;
                    is >> doubleValue;
                    node.input_values[key] = doubleValue;
                }
                else if (value_str == "float")
                {
                    float doubleValue;
                    is >> doubleValue;
                    node.input_values[key] = doubleValue;
                }
                else if (value_str == "string")
                {
                    std::string stringValue;
                    is >> stringValue;
                    node.input_values[key] = stringValue;
                } // Add more types as needed
            }
        }

        // Deserialize internal_values
        int num_internal_values;
        is >> num_internal_values;
        for (int j = 0; j < num_internal_values; ++j)
        {
            std::string value_str;
            is >> value_str;

            if (value_str == "null")
            {
                // Handle null value
                node.internal_values.push_back(std::any());
            }
            else
            {
                // Deserialize based on type
                if (value_str == "int")
                {
                    int intValue;
                    is >> intValue;
                    node.internal_values.push_back(intValue);
                }
                else if (value_str == "double")
                {
                    double doubleValue;
                    is >> doubleValue;
                    node.internal_values.push_back(doubleValue);
                }
                else if (value_str == "float")
                {
                    float doubleValue;
                    is >> doubleValue;
                    node.internal_values.push_back(doubleValue);
                }
                else if (value_str == "string")
                {
                    std::string stringValue;
                    is >> stringValue;
                    node.internal_values.push_back(stringValue);
                } // Add more types as needed
            }
        }

        graph.nodes[i] = node;
    }

    // Deserialize connections
    int num_connections;
    is >> num_connections;
    is.ignore(); // Ignore the newline character after reading num_connections
    for (int i = 0; i < num_connections; ++i)
    {
        GraphInfo::connection connection;
        is >> connection.node_from_id >> connection.node_to_id;
        is >> connection.pin_from_number >> connection.pin_to_number;
        graph.connections[i] = connection;
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

    void recover(GraphInfo info, TypesRecoverFactory *factory)
    {
        for (auto &[id, info] : info.nodes)
        {
            auto node = factory->getNode(info.type_id);
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

    GraphInfo get_info(NodeEditorComponent *editor_component)
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
            node_info.input_values = node->getInputValues();
            node_info.internal_values = node->getInternalValues();
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