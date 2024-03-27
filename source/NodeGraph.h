#pragma once
#include "string"
#include "vector"
#include <stdexcept>

enum PinType{
    Signal, Control
};
class Node;
class ConnectionFactory;
class Pin{
public:
    Pin(int _number, std::string _name, PinType _type, Node *_node);
    std::string name;
    PinType type;
    Node* node;
    int number;
    virtual bool isInput() = 0;
    virtual void accept(ConnectionFactory * factory) = 0;
};
class Input: public Pin{
    public:
    Input(int _number, std::string _name, PinType _type, Node *_node):Pin(_number, _name,  _type, _node){};
    bool isInput() override;
    void accept(ConnectionFactory * factory) override;
};
class Output: public Pin{
    public:
    Output(int _number, std::string _name, PinType _type, Node *_node):Pin(_number, _name,  _type, _node){};
    bool isInput() override;
    void accept(ConnectionFactory * factory) override;
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

    void setAsPin(Pin * pin);

};

class ConnectionFactory{
    public:
    ConnectionFactory(){
        input = nullptr;
        output = nullptr;
    };
    void addPin(Pin* pin){
        pin->accept(this);
    }
    Connection * createConnection(){
        if(input == nullptr || output == nullptr){
            throw std::invalid_argument("Pins are not valid");
        }
        if((int)output->isInput() + (int)input->isInput() != 1 || output->type != input->type){
            throw std::invalid_argument("Pins are not valid");
        }
        return new Connection(output,input);
    }
    Input* input;
    Output* output;
};


class Graph{
    public:
    Graph();
    std::vector<Node*> getNodes();
    std::vector<Connection*> getConnections();
    ~Graph();
    void addNode(Node * node);
    Connection * addConnection(Pin * pin1, Pin * pin2);

    private:
    std::vector<Node*> nodes;
    std::vector<Connection*> connections;
    int getId();
    int id;
};

