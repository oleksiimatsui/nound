#pragma once
#include <unordered_map>
#include <map>
#include "string"
#include "vector"
#include <stdexcept>

enum PinType
{
    Signal,
    Control
};
class Node;
class ConnectionBuilder;
class Pin
{
public:
    Pin(){};
    Pin(int _key, std::string _name, PinType _type, Node *_node);
    std::string name;
    PinType type;
    Node *node;
    int key;
    virtual bool isInput() = 0;
    virtual void accept(ConnectionBuilder *factory) = 0;
};
class Input : public Pin
{
public:
    Input(){};
    Input(int _key, std::string _name, PinType _type, Node *_node);
    bool isInput() override;
    void accept(ConnectionBuilder *factory) override;
};
class Output : public Pin
{
public:
    Output(){};
    Output(int _key, std::string _name, PinType _type, Node *_node);
    bool isInput() override;
    void accept(ConnectionBuilder *factory) override;
};

#include <any>
using Data = std::any;
class Graph;
class Node
{
public:
    Node();
    std::string header;
    std::map<int, Output> outputs = {};
    std::map<int, Input> inputs = {};
    int id;
    Graph *graph;
    void triggerAsync(Data *v, [[maybe_unused]] Input *pin);

protected:
    void virtual trigger(Data *v, [[maybe_unused]] Input *pin);
    void registerInput(int key, const std::string &name, PinType type);
    void registerOutput(int key, const std::string &name, PinType type);
};

class Connection
{
public:
    Connection(Output *from, Input *to);
    int getNodeFromId();
    int getNodeToId();
    int getPinFromNumber();
    int getPinToNumber();

    int id;
    Output *pin_from;
    Input *pin_to;

    void setAsPin(Pin *pin);
};

class ConnectionBuilder
{
public:
    ConnectionBuilder();
    void addPin(Pin *pin);
    Connection *build();
    Input *input;
    Output *output;
};

class GraphListener
{
public:
    virtual void NodeAdded(Node *node) = 0;
    virtual void NodeDeleted(int id) = 0;
    virtual void ConnectionAdded(Connection *connection) = 0;
    virtual void ConnectionDeleted(int id) = 0;
    virtual void message(std::string text) = 0;
};

class Graph
{
public:
    Graph();
    std::unordered_map<int, Node *> getNodes();
    std::unordered_map<int, Connection *> getConnections();
    ~Graph();
    void addNode(Node *node);
    Connection *addConnection(Pin *pin1, Pin *pin2);
    std::vector<int> getConnectionsOfNode(int id);
    void deleteConnection(int id);
    void deleteNode(int id);

    void registerListener(GraphListener *listener);

    void triggerPin(Output *pin, Data *data);

private:
    std::unordered_map<int, Node *> nodes;
    std::unordered_map<int, Connection *> connections;
    int getId();
    int id;
    std::vector<GraphListener *> listeners;
};
