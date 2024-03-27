#include "NodeGraph.h"



Pin::Pin(int _number, std::string _name, PinType _type, Node *_node)
    :number(_number), name(_name), type(_type), node(_node){}


bool Input::isInput() {
        return true;
    }

bool Output::isInput(){
        return false;
    }


Node::Node(){};


Connection::Connection(Output * from, Input * to){
        pin_from = from;
        pin_to = to;
    };
int Connection::getNodeFromId(){
        return pin_from->node->id;
};
    int Connection::getNodeToId(){
        return pin_to->node->id;
};
    int Connection::getPinFromNumber(){
        return pin_from->number;
};
    int Connection::getPinToNumber(){
        return pin_to->number;
};


Graph::Graph(){
        id = 0;
    };
    std::vector<Node*> Graph::getNodes(){
        return nodes;
    };
    std::vector<Connection*> Graph::getConnections(){
        return connections;
    };
    Graph::~Graph(){
        for (auto& n : nodes) delete n;
            nodes.clear();
    };
    void Graph::addNode(Node * node){
        node->id = getId();
        nodes.push_back(node);
    };

    int Graph::addConnection(Connection * connection){
        connection->id = getId();
        connections.push_back(connection);
        return id;
    };

    int Graph::getId(){
        id++;
        return id;
    }