#include "NodeGraph.h"

Pin::Pin(int _key, std::string _name, int _type, Node *_node)
    : key(_key), name(_name), type(_type), node(_node) {}
Output::Output(int _key, std::string _name, int _type, Node *_node) : Pin(_key, _name, _type, _node){};
Input::Input(int _key, std::string _name, int _type, Node *_node) : Pin(_key, _name, _type, _node){};

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

Node::Node()
{
    graph = nullptr;
    id = -1;
};
Node::~Node()
{
    for (auto &[_, n] : inputs)
        delete n;
    inputs.clear();
    for (auto &[_, n] : outputs)
        delete n;
    outputs.clear();
};

void Node::trigger(Value &d, [[maybe_unused]] Input *pin) {

};
void Node::registerInput(int key, const std::string &name, int type)
{
    inputs[key] = new Input(key, name, type, this);
    // inputs.push_back(Input(inputs.size(), name, type, this));
};
void Node::registerOutput(int key, const std::string &name, int type)
{
    outputs[key] = new Output(key, name, type, this);
};

Connection::Connection(Output *from, Input *to)
{
    pin_from = from;
    pin_to = to;
    id = -1;
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
    return pin_from->key;
};
int Connection::getPinToNumber()
{
    return pin_to->key;
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
    node->graph = this;
    node->id = getId();
    nodes[id] = node;
    for (auto &l : listeners)
    {
        l->NodeAdded(node);
    };
};

void Graph::triggerPin(Output *pin, Value &data)
{
    for (auto &[id, c] : connections)
    {
        if (c->pin_from == pin)
        {
            auto node = c->pin_to->node;
            node->trigger(data, c->pin_to);
            for (auto &l : listeners)
            {
                l->message("connection " + std::to_string(id) + " is triggered");
            };
        }
    }
};

std::vector<Input *> Graph::getInputsOfOutput(Output *pin)
{
    std::vector<Input *> pins;
    for (auto &[id, c] : connections)
    {
        if (c->pin_from == pin)
        {
            pins.push_back(c->pin_to);
        }
    }
    return pins;
}

Connection *Graph::addConnection(Pin *pin1, Pin *pin2)
{
    ConnectionBuilder factory;
    factory.addPin(pin1);
    factory.addPin(pin2);
    Connection *connection;
    connection = factory.build();
    connection->id = getId();
    connections[id] = (connection);
    for (auto &l : listeners)
    {
        l->ConnectionAdded(connection);
    };
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
std::vector<int> Graph::getInputConnectionsOfNode(int node_id)
{
    std::vector<int> ids;
    for (auto &[id, c] : connections)
    {
        if (c->getNodeToId() == node_id)
        {
            ids.push_back(id);
        }
    }

    return ids;
};

void Graph::deleteConnection(int id)
{
    connections.erase(id);
    for (auto &l : listeners)
    {
        l->ConnectionDeleted(id);
    };
}
void Graph::deleteNode(int id)
{
    auto connections_ids = getConnectionsOfNode(id);
    for (auto &con_id : connections_ids)
    {
        Graph::deleteConnection(con_id);
    };

    delete nodes[id];
    nodes.erase(id);

    for (auto &l : listeners)
    {
        l->NodeDeleted(id);
    };
}

void Graph::registerListener(GraphListener *listener)
{
    listeners.push_back(listener);
}
