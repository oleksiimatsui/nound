#pragma once
#include "NodeGraph.h"
#include "EditorNode.h"
#include <JuceHeader.h>
#include "GraphProvider.h"
#include "vector"
#include "Theme.h"
#include "Components.h"
#include "Sources.h"

class OutputNode : public EditorNode
{
public:
    enum InputKeys
    {
        audio_
    };
    OutputNode() : EditorNode()
    {
        header = "Speaker";
        inputs[InputKeys::audio_] = new Input(InputKeys::audio_, "audio", PinType::Audio, this);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }
    // SoundOutputSource source;
    StartableSource *result;

private:
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::audio_])
        {
            result = std::any_cast<StartableSource *>(data);
            //  source.setSource(f);
            //  source.Start();
        }
    }
};

class FileReader : public EditorNode, public FileInputListener
{
public:
    enum InputKeys
    {
        predecessor
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
    void setFile(juce::URL *resource, std::string _name) override
    {
        currentAudioFile = resource;
        name = _name;
    };

private:
    std::string name;
    juce::URL *currentAudioFile;
    FileInput *internal;
    std::vector<StartableSource *> sources;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::predecessor])
        {
        }
        auto connection_inputs = graph->getInputsOfOutput(outputs[OutputKeys::audio_]);
        for (auto &input : connection_inputs)
        {
            auto fs = new FileSource();
            fs->setFile(name);
            sources.push_back(fs);
        }
        for (int i = 0; i < connection_inputs.size(); i++)
        {
            auto input = connection_inputs[i];
            input->node->trigger((Data)sources[i], input);
        }
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
            input->node->trigger(source, input);
        }
    }
};

class RandomNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        seconds_
    };
    enum OutputKeys
    {
        audio_out
    };
    RandomNode()
    {
        header = "Random";
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInputWithComponent(InputKeys::seconds_, "seconds", PinType::Number, new NumberInput(this, 0, 5000, &(t)));
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    float t;
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::seconds_])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)internals[InputKeys::seconds_])->update();
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new RandomSource(t);
            Data source = (StartableSource *)(fs);
            input->node->trigger(source, input);
        }
    }
};

class WaveformNode : public EditorNode, ValueHolder<Waveform>
{
public:
    enum InputKeys
    {
    };
    enum OutputKeys
    {
        wave_out
    };
    enum Operations
    {
        sine = 1,
        square,
        sawtooth,
        triangle
    };
    void operationChanged()
    {
        Waveform *new_wave;
        switch (c->getSelectedId())
        {
        case Operations::sine:
            new_wave = new Sine();
            break;
        case Operations::square:
            new_wave = new Square();
            break;
        case Operations::sawtooth:
            new_wave = new Sawtooth();
            break;
        case Operations::triangle:
            new_wave = new Triangle();
            break;
        }
        waveform = new_wave;
    };
    WaveformNode()
    {
        header = "Waveform";
        waveform = new Sine();
        registerOutput(OutputKeys::wave_out, "wave", PinType::Waveform);
        c = new juce::ComboBox();
        c->setSize(1000, 20);
        c->addItem("Sine", Operations::sine);
        c->addItem("Square", Operations::square);
        c->addItem("Sawtooth", Operations::sawtooth);
        c->addItem("Triangle", Operations::triangle);
        c->setSelectedId(Operations::sine);
        c->onChange = [this]
        { operationChanged(); };
    };
    juce::Component *getInternal() override
    {
        return c;
    }
    Waveform *waveform;
    Waveform *getState() override
    {
        return waveform;
    }

private:
    juce::ComboBox *c;
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::wave_out]))
        {
            Data source = (ValueHolder<Waveform> *)(this);
            input->node->trigger(source, input);
        }
    }
};

class OscillatorNode : public EditorNode, NumberInput::Listener, ValueHolder<Waveform>
{
public:
    enum InputKeys
    {
        frequency_,
        phase_,
        osc_,
        seconds_,
    };
    enum OutputKeys
    {
        audio_out
    };
    Waveform *getState()
    {
        return new Sine();
    }

    OscillatorNode() : EditorNode()
    {
        osc = this;
        header = "Oscillator";
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::osc_, "wave", PinType::Waveform);
        registerInputWithComponent(InputKeys::frequency_, "frequency", PinType::Number, new NumberInput(this, 0, 5000, &(frequency)));
        registerInputWithComponent(InputKeys::phase_, "phase", PinType::Number, new NumberInput(this, 0, 5000, &(phase)));
        registerInputWithComponent(InputKeys::seconds_, "seconds", PinType::Number, new NumberInput(this, 0, 5000, &(t)));
    };

    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    float frequency = 440;
    float phase = 0;
    float t = 1;
    ValueHolder<Waveform> *osc;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::frequency_])
        {
            frequency = std::any_cast<float>(data);
            ((NumberInput *)internals[InputKeys::frequency_])->update();
        }
        if (pin == inputs[InputKeys::seconds_])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)internals[InputKeys::seconds_])->update();
        }
        if (pin == inputs[InputKeys::osc_])
        {
            osc = std::any_cast<ValueHolder<Waveform> *>(data);
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new Osc(t, frequency, phase, *osc);
            Data source = (StartableSource *)(fs);
            input->node->trigger(source, input);
        }
    }
};

class AudioMathNode : public EditorNode, public ValueHolder<MathAudioSource::State>
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
        states[Operations::add] = new MathAudioSource::Add();
        states[Operations::divide] = new MathAudioSource::Divide();
        states[Operations::substract] = new MathAudioSource::Substract();
        states[Operations::multiply] = new MathAudioSource::Multiply();

        s1 = nullptr;
        s2 = nullptr;
        c = nullptr;
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
        //  state = &states[(Operations)c->getSelectedId()];
        switch (c->getSelectedId())
        {
        case Operations::add:
            state = states[Operations::add];
            break;
        case Operations::substract:
            state = states[Operations::substract];
            break;
        case Operations::multiply:
            state = states[Operations::multiply];
            break;
        case Operations::divide:
            state = states[Operations::divide];
            break;
        }
    };
    std::unordered_map<Operations, MathAudioSource::State *> states;
    MathAudioSource::State *getState() override
    {
        return state;
    }

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
                    s->stateholder = this;
                    Data source = (StartableSource *)(s);
                    input->node->trigger(source, input);
                }
            }
        }
    }
};

class FMNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        modulator_,
        frequency_,
        depth_,
        wave_
    };
    enum OutputKeys
    {
        audio_out
    };
    FMNode()
    {
        header = "Frequency modulator";
        registerInput(InputKeys::modulator_, "modulator", PinType::Audio);
        registerInputWithComponent(InputKeys::frequency_, "frequency", PinType::Number, new NumberInput(this, 0, 5000, &(frequency)));
        registerInput(InputKeys::wave_, "waveform", PinType::Waveform);
        registerInputWithComponent(InputKeys::depth_, "depth", PinType::Number, new NumberInput(this, 0, 5000, &(depth)));
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    float frequency = 440;
    float depth = 100;
    StartableSource *s;
    Data datatosend;
    Vertical internal;
    ValueHolder<Waveform> *wf = nullptr;
    std::vector<Input *> listeners;
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::frequency_])
        {
            frequency = std::any_cast<float>(data);
            ((NumberInput *)internals[InputKeys::frequency_])->update();
        }
        if (pin == inputs[InputKeys::wave_])
        {
            wf = std::any_cast<ValueHolder<Waveform> *>(data);
            ((NumberInput *)internals[InputKeys::depth_])->update();
        }
        if (pin == inputs[InputKeys::depth_])
        {
            depth = std::any_cast<float>(data);
            ((NumberInput *)internals[InputKeys::depth_])->update();
        }

        if (pin == inputs[InputKeys::modulator_])
        {
            if (StartableSource *f = std::any_cast<StartableSource *>(data))
            {
                s = f;
            }
        }

        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new FM(frequency, depth, s, wf);
            Data source = (StartableSource *)(fs);
            input->node->trigger(source, input);
        }
    }
};

class NumberMathNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        number_1,
        number_2
    };
    enum OutputKeys
    {
        number_out
    };
    NumberMathNode()
    {
        states[Operations::add] = new MathAudioSource::Add();
        states[Operations::divide] = new MathAudioSource::Divide();
        states[Operations::substract] = new MathAudioSource::Substract();
        states[Operations::multiply] = new MathAudioSource::Multiply();
        c = nullptr;
        header = "Number Math";
        registerInputWithComponent(InputKeys::number_1, "number", PinType::Number, new NumberInput(this, 0, 5000, &(val1)));
        registerInputWithComponent(InputKeys::number_2, "number", PinType::Number, new NumberInput(this, 0, 5000, &(val2)));
        registerOutput(OutputKeys::number_out, "number", PinType::Number);
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
        //  state = &states[(Operations)c->getSelectedId()];
        switch (c->getSelectedId())
        {
        case Operations::add:
            state = states[Operations::add];
            break;
        case Operations::substract:
            state = states[Operations::substract];
            break;
        case Operations::multiply:
            state = states[Operations::multiply];
            break;
        case Operations::divide:
            state = states[Operations::divide];
            break;
        }
    };
    std::unordered_map<Operations, MathAudioSource::State *> states;
    void valueChanged() override
    {
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::number_out]))
        {
            Data source = state->operation(val1, val2);
            input->node->trigger(source, input);
        }
    };

private:
    juce::ComboBox *c;
    MathAudioSource::State *state;

    float val1 = 0;
    float val2 = 0;
    float res;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        if (pin != nullptr)
        {
            if (float f = std::any_cast<float>(data))
            {
                if (pin == inputs[InputKeys::number_1])
                {
                    val1 = f;
                    ((NumberInput *)internals[InputKeys::number_1])->update();
                }
                else if (pin == inputs[InputKeys::number_2])
                {
                    val2 = f;
                    ((NumberInput *)internals[InputKeys::number_2])->update();
                }
            }
        }

        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::number_out]))
        {
            Data source = state->operation(val1, val2);
            input->node->trigger(source, input);
        }
    }
};

class Concatenate : public EditorNode
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
    Concatenate()
    {
        s1 = nullptr;
        s2 = nullptr;
        header = "Audio Concatenate";
        registerInput(InputKeys::audio_1, "audio", PinType::Audio);
        registerInput(InputKeys::audio_2, "audio", PinType::Audio);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
    };

    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
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
                    auto s = new ConcatenationSource();
                    s->sources = std::vector<StartableSource *>({s1, s2});
                    Data source = (StartableSource *)(s);
                    input->node->trigger(source, input);
                }
            }
        }
    }
};