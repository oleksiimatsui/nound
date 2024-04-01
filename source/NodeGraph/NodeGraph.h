#pragma once
#include <unordered_map>
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
    Pin(int _number, std::string _name, PinType _type, Node *_node);
    std::string name;
    PinType type;
    Node *node;
    int number;
    virtual bool isInput() = 0;
    virtual void accept(ConnectionBuilder *factory) = 0;
};
class Input : public Pin
{
public:
    Input(int _number, std::string _name, PinType _type, Node *_node) : Pin(_number, _name, _type, _node){};
    bool isInput() override;
    void accept(ConnectionBuilder *factory) override;
};
class Output : public Pin
{
public:
    Output(int _number, std::string _name, PinType _type, Node *_node) : Pin(_number, _name, _type, _node){};
    bool isInput() override;
    void accept(ConnectionBuilder *factory) override;
};

class Internal
{
};

#include <any>
using Data = std::any;

class Node
{
public:
    Node();
    std::string header;
    std::vector<Output> outputs;
    std::vector<Internal> internals;
    std::vector<Input> inputs;
    int id;
    void triggerAsync(Data *v, Pin *pin);

private:
    void virtual trigger(Data *v, Pin *pin);

protected:
    void registerInput(std::string name, PinType type);
    void registerOutput(std::string name, PinType type);
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

private:
    std::unordered_map<int, Node *> nodes;
    std::unordered_map<int, Connection *> connections;
    int getId();
    int id;
    std::vector<GraphListener *> listeners;
};
