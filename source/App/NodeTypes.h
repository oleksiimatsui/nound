#pragma once
#include "NodeGraph.h"
#include <JuceHeader.h>
#include "vector"

class NodeFactory
{
public:
    virtual Node *create() = 0;
};

class StartNode : public Node
{
public:
    StartNode() : Node()
    {
        header = "Start";
        registerOutput("trigger", PinType::Control);
    };

private:
    void trigger(Data *data, Pin *pin) override
    {
        for (auto &p : outputs)
        {
            // p.trigger(data);
        }
    }
};
class StartNodeFactory : public NodeFactory
{
public:
    Node *create() override
    {
        return new StartNode;
    }
};

class SpeakerNode : public Node
{
public:
    SpeakerNode() : Node()
    {
        header = "Speaker";
        registerInput("numbers", PinType::Signal);
    };
};
class SpeakerNodeFactory : public NodeFactory
{
public:
    Node *create() override
    {
        return new SpeakerNode;
    }
};

class FileReader : public Node
{
public:
    FileReader() : Node()
    {
        header = "File Reader";
        registerInput("trigger", PinType::Control);
        registerOutput("end", PinType::Control);
        registerOutput("numbers", PinType::Signal);
    };
};
class FileReaderFactory : public NodeFactory
{
public:
    Node *create() override
    {
        return new FileReader;
    }
};