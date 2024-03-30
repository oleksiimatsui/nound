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
        Output output(0, "trigger", PinType::Control, this);
        outputs.push_back(output);
    };
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
        Input input(0, "numbers", PinType::Signal, this);
        inputs.push_back(input);
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
        inputs.push_back(Input(0, "trigger", PinType::Control, this));
        outputs.push_back(Output(0, "numbers", PinType::Signal, this));
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