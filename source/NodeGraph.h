#pragma once
#include "string"
#include "vector"

class InternalUIData{

};
enum PinType{
    Signal, Control
};

class Pin{
public:
    std::string name;
    PinType type;
};
class Input: public Pin{

};
class Output: public Pin{

};
class Internal{
    public:
    InternalUIData data;
};

class NodeDepictor{
    public:
    std::string header;
};

class Node{
    public:
    Node(){
    };
    std::string header;
    std::vector<Output> outputs;
    std::vector<Internal> internals;
    std::vector<Input> inputs;
    int id;
};


class Graph{
    public:
    Graph(){
        id = 0;
    };
    std::vector<Node*> getNodes(){
        return nodes;
    };
    ~Graph(){
        for (auto& n : nodes) delete n;
            nodes.clear();
    };
    void addNode(Node * node){
        node->id = getId();
        nodes.push_back(node);
    };

    private:
    std::vector<Node*> nodes;
    int getId(){
        id++;
        return id;
    }
    int id;
};

