#pragma once
#include <unordered_map>
#include <map>
#include "string"
#include "vector"
#include <stdexcept>
#include <future>

class Node;
class ConnectionBuilder;
class Pin
{
public:
    Pin(){};
    Pin(int _key, std::string _name, int _type, Node *_node);
    std::string name;
    int type;
    Node *node;
    int key;
    virtual bool isInput() = 0;
    virtual void accept(ConnectionBuilder *factory) = 0;
};
class Input : public Pin
{
public:
    Input(){};
    Input(int _key, std::string _name, int _type, Node *_node);
    bool isInput() override;
    void accept(ConnectionBuilder *factory) override;
};
class Output : public Pin
{
public:
    Output(){};
    Output(int _key, std::string _name, int _type, Node *_node);
    bool isInput() override;
    void accept(ConnectionBuilder *factory) override;
};

#include <any>
using Value = std::any;
class Graph;
class Node
{
public:
    Node();
    virtual ~Node();
    std::string header;
    std::map<int, Output *> outputs = {};
    std::map<int, Input *> inputs = {};
    int id;
    Graph *graph;
    void virtual trigger(Value &v, [[maybe_unused]] Input *pin);

protected:
    //  void virtual trigger(Value &v, [[maybe_unused]] Input *pin);
    void registerInput(int key, const std::string &name, int type);
    void registerOutput(int key, const std::string &name, int type);
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
    virtual ~Graph();
    void addNode(Node *node);
    Connection *addConnection(Pin *pin1, Pin *pin2);
    std::vector<int> getConnectionsOfNode(int id);
    std::vector<int> getInputConnectionsOfNode(int id);
    void deleteConnection(int id);
    void deleteNode(int id);
    void registerListener(GraphListener *listener);
    void triggerPin(Output *pin, Value &Value);
    std::vector<Input *> getInputsOfOutput(Output *pin);
    int getOutputsOfInputSize(Input *pin);

protected:
    std::unordered_map<int, Node *> nodes;
    std::unordered_map<int, Connection *> connections;
    int getId();
    int auto_increment;
    std::vector<GraphListener *> listeners;
};