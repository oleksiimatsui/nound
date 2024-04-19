#pragma once
#include "NodeGraph.h"
#include "EditorNode.h"
#include <JuceHeader.h>
#include "GraphProvider.h"
#include "vector"
#include "Theme.h"
#include "Components.h"
#include "Sources.h"

// class StartNode : public EditorNode
// {
// public:
//     enum OutputKeys
//     {

//     };
//     StartNode() : EditorNode()
//     {
//         header = "Start";
//     };
//     juce::Component *getInternal() override
//     {
//         return nullptr;
//     }

// private:
//     void trigger(Data &data, [[maybe_unused]] Input *pin) override
//     {
//         for (auto &[_, p] : outputs)
//         {
//             for (auto &input : graph->getInputsOfOutput(p))
//             {
//                 input->node->triggerAsync(data, input);
//             }
//         }
//     }
// };

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
        inputs[InputKeys::audio_] = new Input(InputKeys::audio_, "audio", PinType::Audio, this);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }
    SoundOutputSource source;

private:
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::audio_])
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
    };
    enum OutputKeys
    {
        audio_
    };
    FileReader() : EditorNode()
    {
        currentAudioFile = nullptr;
        internal = new FileInput(this);
        header = "File Reader";
        outputs[OutputKeys::audio_] = new Output(OutputKeys::audio_, "audio", PinType::Audio, this);
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
    std::string name;
    juce::URL *currentAudioFile;
    FileInput *internal;
    FileSource filesource;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_]))
        {
            auto fs = new FileSource();
            fs->setFile(name);
            Data source = (StartableSource *)(fs);
            input->node->triggerAsync(source, input);
        }
    }
};

class WaitNode : public EditorNode
{
public:
    enum InputKeys
    {
        // trigger_in
    };
    enum OutputKeys
    {
        // trigger_out
    };
    WaitNode() : EditorNode()
    {
        header = "Wait";
        // registerInput(InputKeys::trigger_in, "trigger", PinType::Number);
        // registerOutput(OutputKeys::trigger_out, "trigger", PinType::Number);
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
        // for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::trigger_out]))
        // {
        //     input->node->triggerAsync(source, input);
        // }
    }
};

class ReverbNode : public EditorNode, public NumberInput::Listener
{
public:
    enum InputKeys
    {
        audio_in,
        width,
        dryLevel,
        damping,
        freezeMode,
        roomSize,
        wetLevel
    };
    enum OutputKeys
    {
        audio_out
    };
    void valueChanged() override
    {
        for (auto &r : results)
        {
            r->r->setParameters(p);
        }
    }

    ReverbNode() : EditorNode()
    {
        int i = 0;
        int h = ThemeProvider::getCurrentTheme()->nodeTextHeight;
        header = "Reverb";
        registerInput(InputKeys::audio_in, "audio", PinType::Audio);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInputWithComponent(InputKeys::width, "width", PinType::Number, new NumberInput(this, 0, 1, &(p.width)));
        registerInputWithComponent(InputKeys::damping, "damping", PinType::Number, new NumberInput(this, 0, 1, &(p.damping)));
        registerInputWithComponent(InputKeys::dryLevel, "dryLevel", PinType::Number, new NumberInput(this, 0, 1, &(p.dryLevel)));
        registerInputWithComponent(InputKeys::freezeMode, "freezeMode", PinType::Number, new NumberInput(this, 0, 1, &(p.freezeMode)));
        registerInputWithComponent(InputKeys::roomSize, "roomSize", PinType::Number, new NumberInput(this, 0, 1, &(p.roomSize)));
        registerInputWithComponent(InputKeys::wetLevel, "wetLevel", PinType::Number, new NumberInput(this, 0, 1, &(p.wetLevel)));

        valueChanged();
    };

    ~ReverbNode()
    {
        for (auto &n : results)
            delete n;
        results.clear();
    }

    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    juce::Reverb::Parameters p;
    std::vector<ReverbSource *> results;
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &n : results)
            delete n;
        results.clear();
        StartableSource *input_source = std::any_cast<StartableSource *>(data);
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new ReverbSource();
            results.push_back(fs);
            fs->setSource(input_source);
            fs->r->setParameters(p);
            Data source = (StartableSource *)(fs);
            input->node->triggerAsync(source, input);
        }
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
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new RandomSource();
            Data source = (StartableSource *)(fs);
            input->node->triggerAsync(source, input);
        }
    }
};

class SineNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        frequency_
    };
    enum OutputKeys
    {
        audio_out
    };

    SineNode() : EditorNode()
    {
        header = "Sine";
        registerInputWithComponent(InputKeys::frequency_, "frequency", PinType::Number, new NumberInput(this, 0, 5000, &(frequency)));
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
    };

    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    float frequency = 440;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new SineSource(frequency);
            Data source = (StartableSource *)(fs);
            input->node->triggerAsync(source, input);
        }
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
        s1 = nullptr;
        s2 = nullptr;
        header = "Audio Math";
        registerInput(InputKeys::audio_1, "audio", PinType::Audio);
        registerInput(InputKeys::audio_2, "audio", PinType::Audio);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
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
        c->setSelectedId(Operations::add);
        c->onChange = [this]
        { operationChanged(); };
        return c;
    }
    void operationChanged()
    {
        switch (c->getSelectedId())
        {
        case Operations::add:
            state = new MathAudioSource::Add();
            break;
        case Operations::substract:
            state = new MathAudioSource::Substract();
            break;
        case Operations::multiply:
            state = new MathAudioSource::Multiply();
            break;
        case Operations::divide:
            state = new MathAudioSource::Divide();
            break;
        }
    };

private:
    juce::ComboBox *c;
    MathAudioSource::State *state;
    StartableSource *s1;
    StartableSource *s2;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (StartableSource *f = std::any_cast<StartableSource *>(data))
        {
            if (pin == inputs[InputKeys::audio_1])
            {
                s1 = f;
            }
            else if (pin == inputs[InputKeys::audio_2])
            {
                s2 = f;
            }
            if (s1 != nullptr && s2 != nullptr)
            {
                for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
                {
                    auto s = new MathAudioSource();
                    s->s1 = s1;
                    s->s2 = s2;
                    Data source = (StartableSource *)(s);
                    input->node->triggerAsync(source, input);
                }
            }
        }
    }
};

class AudioToNumberNode : public EditorNode
{
public:
    enum InputKeys
    {
        audio_in
    };
    enum OutputKeys
    {
        number_out
    };
    AudioToNumberNode()
    {
        header = "AudioToNumbers";
        registerInput(InputKeys::audio_in, "audio", PinType::Number);
        registerOutput(OutputKeys::number_out, "number", PinType::Audio);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    Data datatosend;
    Vertical internal;
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {

        // graph->triggerPin(&outputs[OutputKeys::audio_out], datatosend);
    }
};