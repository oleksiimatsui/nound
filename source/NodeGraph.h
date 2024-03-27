#pragma once
#include "string"
#include "vector"

enum PinType{
    Signal, Control
};
class Node;
class Pin{
public:
    Pin(int _number, std::string _name, PinType _type, Node *_node);
    std::string name;
    PinType type;
    Node* node;
    int number;
    virtual bool isInput() = 0;
};
class Input: public Pin{
    public:
    Input(int _number, std::string _name, PinType _type, Node *_node):Pin(_number, _name,  _type, _node){};
    bool isInput() override;
};
class Output: public Pin{
    public:
    Output(int _number, std::string _name, PinType _type, Node *_node):Pin(_number, _name,  _type, _node){};
    bool isInput() override;
};

class Internal{

};

class Node{
    public:
    Node();
    std::string header;
    std::vector<Output> outputs;
    std::vector<Internal> internals;
    std::vector<Input> inputs;
    int id;
};

class Connection{
    public:
    Connection(Output * from, Input * to);
    int getNodeFromId();
    int getNodeToId();
    int getPinFromNumber();
    int getPinToNumber();

    int id;
    Output * pin_from;
    Input * pin_to;

};


class Graph{
    public:
    Graph();
    std::vector<Node*> getNodes();
    std::vector<Connection*> getConnections();
    ~Graph();
    void addNode(Node * node);
    int addConnection(Connection * connection);

    private:
    std::vector<Node*> nodes;
    std::vector<Connection*> connections;
    int getId();
    int id;
};

