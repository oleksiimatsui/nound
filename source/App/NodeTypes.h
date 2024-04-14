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
    SoundOutputSource source;

private:
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == &inputs[InputKeys::audio_])
        {
            StartableSource *f = std::any_cast<StartableSource *>(data);
            source.setSource(f);
            source.Start();
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
        bool isStart = std::any_cast<bool>(data);
        if (isStart)
        {
            source = (StartableSource *)&filesource;
            graph->triggerPin(&outputs[OutputKeys::audio_], source);
        }
        else
        {
            source = (StartableSource *)&filesource;
            filesource.Stop();
        }
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
    }

private:
    Data source;
    int milliseconds = 5000;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        source = true;
        juce::Time::waitForMillisecondCounter(milliseconds);
        graph->triggerPin(&outputs[OutputKeys::trigger_out], source);
    }
};

class ReverbNode : public EditorNode, public juce::Slider::Listener
{
public:
    enum InputKeys
    {
        audio_in
    };
    enum OutputKeys
    {
        audio_out
    };
    void sliderValueChanged(juce::Slider *slider) override
    {
        double val = slider->getValue();
    }

    ReverbNode() : EditorNode()
    {
        internal.setRange(0, 1);         // [1]
        internal.setTextValueSuffix(""); // [2]
        internal.setValue(0.5);
        internal.addListener(this);
        internal.setSize(1000, 20);
        header = "Reverb";
        registerInput(InputKeys::audio_in, "audio", PinType::Signal);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return &internal;
    }

private:
    Data source;
    juce::Slider internal;
    ReverbSource reverb_source;
    juce::Reverb::Parameters p;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {

        StartableSource *input_source = std::any_cast<StartableSource *>(data);

        reverb_source.setSource(input_source);
        //  p.width = 1;
        //  reverb_source->setParameters(p);

        source = (StartableSource *)&reverb_source;
        graph->triggerPin(&outputs[OutputKeys::audio_out], source);
    }
};

class RandomNode : public EditorNode
{
public:
    enum InputKeys
    {
        start
    };
    enum OutputKeys
    {
        audio_out
    };
    RandomNode()
    {
        header = "Random";
        registerInput(InputKeys::start, "start", PinType::Control);

        registerOutput(OutputKeys::audio_out, "audio", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    Data source;
    RandomSource random;
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {

        source = (StartableSource *)(&random);
        graph->triggerPin(&outputs[OutputKeys::audio_out], source);
    }
};

class AudioMathNode : public EditorNode
{
public:
    enum InputKeys
    {
        audio_1,
        audio_2
    };
    enum OutputKeys
    {
        audio_out
    };
    AudioMathNode()
    {
        header = "Audio Math";
        registerInput(InputKeys::audio_1, "audio", PinType::Signal);
        registerInput(InputKeys::audio_2, "audio", PinType::Signal);

        registerOutput(OutputKeys::audio_out, "audio", PinType::Signal);
    };
    enum Operations
    {
        add = 1,
        substract,
        multiply,
        divide
    };
    juce::Component *getInternal() override
    {
        c = new juce::ComboBox();
        c->setSize(1000, 20);
        c->addItem("Add", Operations::add);
        c->addItem("Subtract", Operations::substract);
        c->addItem("Multiply", Operations::multiply);
        c->addItem("Divide", Operations::divide);
        c->setItemEnabled(Operations::add, false);
        c->onChange = [this]
        { operationChanged(); };
        return c;
    }
    void operationChanged()
    {
        switch (c->getSelectedId())
        {
        case Operations::add:
            MAsource.state = new MathAudioSource::Add();
            break;
        case Operations::substract:
            MAsource.state = new MathAudioSource::Substract();
            break;
        case Operations::multiply:
            MAsource.state = new MathAudioSource::Multiply();
            break;
        case Operations::divide:
            MAsource.state = new MathAudioSource::Divide();
            break;
        }
    };

private:
    juce::ComboBox *c;
    Data source;
    MathAudioSource MAsource;
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (StartableSource *f = std::any_cast<StartableSource *>(data))
        {
            if (pin == &inputs[InputKeys::audio_1])
            {
                MAsource.s1 = f;
            }
            else if (pin == &inputs[InputKeys::audio_2])
            {
                MAsource.s2 = f;
            }
            if (MAsource.s1 != nullptr && MAsource.s2 != nullptr)
            {
                source = (StartableSource *)(&MAsource);
                graph->triggerPin(&outputs[OutputKeys::audio_out], source);
            }
        }
    }
};