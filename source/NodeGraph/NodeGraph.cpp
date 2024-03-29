#include "NodeGraph.h"

Pin::Pin(int _number, std::string _name, PinType _type, Node *_node)
    : number(_number), name(_name), type(_type), node(_node) {}

bool Input::isInput()
{
    return true;
}

void Input::accept(ConnectionBuilder *factory)
{
    factory->input = this;
}

bool Output::isInput()
{
    return false;
}

void Output::accept(ConnectionBuilder *factory)
{
    factory->output = this;
}

Node::Node(){};

Connection::Connection(Output *from, Input *to)
{
    pin_from = from;
    pin_to = to;
};
int Connection::getNodeFromId()
{
    return pin_from->node->id;
};
int Connection::getNodeToId()
{
    return pin_to->node->id;
};
int Connection::getPinFromNumber()
{
    return pin_from->number;
};
int Connection::getPinToNumber()
{
    return pin_to->number;
};

ConnectionBuilder::ConnectionBuilder()
{
    input = nullptr;
    output = nullptr;
};
void ConnectionBuilder::addPin(Pin *pin)
{
    pin->accept(this);
}
Connection *ConnectionBuilder::build()
{
    if (input == nullptr || output == nullptr)
    {
        throw std::invalid_argument("Pins are not valid");
    }
    if ((int)output->isInput() + (int)input->isInput() != 1 || output->type != input->type)
    {
        throw std::invalid_argument("Pins are not valid");
    }
    return new Connection(output, input);
}

Graph::Graph()
{
    id = 0;
};
std::unordered_map<int, Node *> Graph::getNodes()
{
    return nodes;
};
std::unordered_map<int, Connection *> Graph::getConnections()
{
    return connections;
};
Graph::~Graph()
{
    for (auto &[_, n] : nodes)
        delete n;
    nodes.clear();
    for (auto &[_, n] : connections)
        delete n;
    connections.clear();
};
void Graph::addNode(Node *node)
{
    node->id = getId();
    nodes[id] = node;
};

Connection *Graph::addConnection(Pin *pin1, Pin *pin2)
{
    ConnectionBuilder factory;
    factory.addPin(pin1);
    factory.addPin(pin2);
    Connection *connection;
    connection = factory.build();
    connection->id = getId();
    connections[id] = (connection);
    return connection;
};

int Graph::getId()
{
    id++;
    return id;
}

std::vector<int> Graph::getConnectionsOfNode(int node_id)
{
    std::vector<int> ids;
    for (auto &[id, c] : connections)
    {
        if (c->getNodeToId() == node_id || c->getNodeFromId() == node_id)
        {
            ids.push_back(id);
        }
    }

    return ids;
};

void Graph::deleteConnection(int id)
{
    connections.erase(id);
}
void Graph::deleteNode(int id)
{
    nodes.erase(id);
}