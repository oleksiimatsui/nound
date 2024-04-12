#pragma once
#include "NodeGraph.h"
#include "EditorNode.h"
#include <JuceHeader.h>
#include "GraphProvider.h"
#include "vector"
#include "Theme.h"
#include "Components.h"
#include "Sources.h"

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
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {

        for (auto &[_, p] : outputs)
        {
            GraphProvider::getGraph()->triggerPin(&p, data);
        }
    }
};

class SpeakerNode : public EditorNode
{
public:
    enum InputKeys
    {
        audio_
    };
    SpeakerNode() : EditorNode()
    {
        header = "Speaker";
        registerInput(InputKeys::audio_, "audio", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }
    SoundOutputSource *source;

private:
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == &inputs[InputKeys::audio_])
        {
            auto f = std::any_cast<FileSource *>(data);
            source = new SoundOutputSource(*f);
            source->Start();
            std::cout << f;
        }
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
        audio_
    };
    FileReader() : EditorNode()
    {
        currentAudioFile = nullptr;
        internal = new FileInput(this);
        header = "File Reader";
        registerInput(InputKeys::trigger_, "trigger", PinType::Control);
        registerOutput(OutputKeys::end_, "end", PinType::Control);
        registerOutput(OutputKeys::audio_, "audio", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return internal;
    }

    ~FileReader()
    {
        delete internal;
    }
    void setFile(juce::URL *resource, std::string name) override
    {
        currentAudioFile = resource;
        filesource.setFile(name);
    };

private:
    Data source;
    juce::URL *currentAudioFile;
    FileInput *internal;
    FileSource filesource;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        source = &filesource;
        graph->triggerPin(&outputs[OutputKeys::audio_], source);
    }
};

class WaitNode : public EditorNode
{
public:
    enum InputKeys
    {
        trigger_in
    };
    enum OutputKeys
    {
        trigger_out
    };
    WaitNode() : EditorNode()
    {
        header = "Wait";
        registerInput(InputKeys::trigger_in, "trigger", PinType::Control);
        registerOutput(OutputKeys::trigger_out, "trigger", PinType::Control);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

    ~WaitNode()
    {
        delete internal;
    }

private:
    Data source;
    FileInput *internal;
    int milliseconds = 5000;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        source = true;
        juce::Time::waitForMillisecondCounter(milliseconds);
        graph->triggerPin(&outputs[OutputKeys::trigger_out], source);
    }
};
