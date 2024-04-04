#pragma once
#include "NodeGraph.h"
#include "EditorNode.h"
#include <JuceHeader.h>
#include "GraphProvider.h"
#include "vector"
#include "Theme.h"
#include "Components.h"

class NodeFactory
{
public:
    virtual EditorNode *create() = 0;
};

class StartNode : public EditorNode
{
public:
    StartNode() : EditorNode()
    {
        header = "Start";
        registerOutput("trigger", PinType::Control);
    };
    juce::Component *getInternal() override
    {
        return new juce::Label("a");
    }

private:
    void trigger(Data *data, [[maybe_unused]] Input *pin) override
    {

        for (auto &p : outputs)
        {
            GraphProvider::getGraph()->triggerPin(&p, data);
        }
    }
};
class StartNodeFactory : public NodeFactory
{
public:
    EditorNode *create() override
    {
        return new StartNode;
    }
};

class SpeakerNode : public EditorNode
{
public:
    SpeakerNode() : EditorNode()
    {
        header = "Speaker";
        registerInput("numbers", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return new juce::Label("a");
    }

private:
    void trigger(Data *data, [[maybe_unused]] Input *pin) override
    {

        for (auto &p : outputs)
        {
            GraphProvider::getGraph()->triggerPin(&p, data);
        }
    }
};
class SpeakerNodeFactory : public NodeFactory
{
public:
    EditorNode *create() override
    {
        return new SpeakerNode;
    }
};

class FileReader : public EditorNode, public FileInputListener
{
public:
    FileReader() : EditorNode()
    {
        internal = new FileInput(this);
        header = "File Reader";
        registerInput("trigger", PinType::Control);
        registerOutput("end", PinType::Control);
        registerOutput("numbers", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return internal;
    }

    ~FileReader()
    {
        delete internal;
    }
    void setFile(URL *resource) override
    {
        currentAudioFile = resource;
    };

private:
    Data source;
    URL *currentAudioFile;
    FileInput *internal;

    void trigger(Data *data, [[maybe_unused]] Input *pin) override
    {
        source = std::make_any<URL *>(currentAudioFile);
        graph->triggerPin(&outputs[1], &source);
    }
};
class FileReaderFactory : public NodeFactory
{
public:
    EditorNode *create() override
    {
        return new FileReader;
    }
};