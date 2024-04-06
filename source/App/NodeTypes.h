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
    enum OutputKeys
    {
        trigger_
    };
    StartNode() : EditorNode()
    {
        header = "Start";
        registerOutput(OutputKeys::trigger_, "trigger", PinType::Control);
    };
    juce::Component *getInternal() override
    {
        return new juce::Label("a");
    }

private:
    void trigger(Data *data, [[maybe_unused]] Input *pin) override
    {

        for (auto &[_, p] : outputs)
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
    enum InputKeys
    {
        numbers_
    };
    SpeakerNode() : EditorNode()
    {
        header = "Speaker";
        registerInput(InputKeys::numbers_, "numbers", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return new juce::Label("a");
    }

private:
    void trigger(Data *data, [[maybe_unused]] Input *pin) override
    {
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
    enum InputKeys
    {
        trigger_
    };
    enum OutputKeys
    {
        end_,
        numbers_
    };
    FileReader() : EditorNode()
    {
        currentAudioFile = nullptr;
        internal = new FileInput(this);
        header = "File Reader";
        registerInput(InputKeys::trigger_, "trigger", PinType::Control);
        registerOutput(OutputKeys::end_, "end", PinType::Control);
        registerOutput(OutputKeys::numbers_, "numbers", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return internal;
    }

    ~FileReader()
    {
        delete internal;
    }
    void setFile(juce::URL *resource) override
    {
        currentAudioFile = resource;
    };

private:
    Data source;
    juce::URL *currentAudioFile;
    FileInput *internal;

    void trigger(Data *data, [[maybe_unused]] Input *pin) override
    {
        source = std::make_any<juce::URL *>(currentAudioFile);
        graph->triggerPin(&outputs[OutputKeys::numbers_], &source);
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