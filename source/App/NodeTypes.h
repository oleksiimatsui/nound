#pragma once
#include "NodeGraph.h"
#include "EditorNode.h"
#include <JuceHeader.h>
#include "vector"
#include "Theme.h"
#include "Components.h"
#include "Sources.h"
#include "RecoverableNodeGraph.h"

enum class NodeTypes
{
    Output,
    FileReader,
    Reverb,
    Waveform,
    AudioMath,
    NumberMath,
    Concatenate,
    FunctionMath,
    Const,
    Random,
    Oscillator
};

class AbstractNodeFactory
{
public:
    virtual EditorNode *create() = 0;
};

template <class T>
class NodeFactory : public AbstractNodeFactory
{
public:
    EditorNode *create() override
    {
        return new T;
    }
};

struct NodeNames
{
    static const std::string OutputNode;
    static const std::string FileReader;
    static const std::string ReverbNode;
    static const std::string RandomNode;
    static const std::string WaveformNode;
    static const std::string Oscillator;
    static const std::string AudioMathNode;
    static const std::string NumberMathNode;
    static const std::string Concatenate;
    static const std::string FunctionMathNode;
    static const std::string ConstNode;
};
const std::string NodeNames::OutputNode = "Output";
const std::string NodeNames::FileReader = "File Reader";
const std::string NodeNames::ReverbNode = "Reverb";
const std::string NodeNames::RandomNode = "Random";
const std::string NodeNames::WaveformNode = "Basic Waveform";
const std::string NodeNames::AudioMathNode = "Audio Math";
const std::string NodeNames::NumberMathNode = "Number Math";
const std::string NodeNames::Concatenate = "Concatenate";
const std::string NodeNames::FunctionMathNode = "Arithmetic";
const std::string NodeNames::ConstNode = "Const";
const std::string NodeNames::Oscillator = "Oscillator";

class OutputNode : public EditorNode
{
public:
    enum InputKeys
    {
        audio_
    };
    OutputNode() : EditorNode()
    {
        header = NodeNames::OutputNode;
        type_id = (int)NodeTypes::Output;
        inputs[InputKeys::audio_] = new Input(InputKeys::audio_, "audio", PinType::Audio, this);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }
    // SoundOutputSource source;
    StartableSource *result;

private:
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::audio_])
        {
            result = std::any_cast<StartableSource *>(data);
            //  source.setSource(f);
            //  source.Start();
        }
    }
};

class FileReaderNode : public EditorNode, public FileInputListener
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
    FileReaderNode() : EditorNode()
    {
        currentAudioFile = nullptr;
        internal = new FileInput(this);
        header = NodeNames::FileReader;
        type_id = (int)NodeTypes::FileReader;
        outputs[OutputKeys::audio_] = new Output(OutputKeys::audio_, "audio", PinType::Audio, this);
    };
    juce::Component *getInternal() override
    {
        return internal;
    }

    ~FileReaderNode()
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

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
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
            input->node->trigger((Value)sources[i], input);
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
        header = NodeNames::ReverbNode;
        type_id = (int)NodeTypes::Reverb;
        registerInput(InputKeys::audio_in, "audio", PinType::Audio);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::width, "width", PinType::Number, new NumberInput(this, 0, 1, &(p.width)), (Value *)&(p.width));
        registerInput(InputKeys::damping, "damping", PinType::Number, new NumberInput(this, 0, 1, &(p.damping)), (Value *)&(p.width));
        registerInput(InputKeys::dryLevel, "dryLevel", PinType::Number, new NumberInput(this, 0, 1, &(p.dryLevel)), (Value *)&(p.width));
        registerInput(InputKeys::freezeMode, "freezeMode", PinType::Number, new NumberInput(this, 0, 1, &(p.freezeMode)), (Value *)&(p.width));
        registerInput(InputKeys::roomSize, "roomSize", PinType::Number, new NumberInput(this, 0, 1, &(p.roomSize)), (Value *)&(p.width));
        registerInput(InputKeys::wetLevel, "wetLevel", PinType::Number, new NumberInput(this, 0, 1, &(p.wetLevel)), (Value *)&(p.width));

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
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
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
            Value source = (StartableSource *)(fs);
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
        header = NodeNames::RandomNode;
        type_id = (int)NodeTypes::Random;
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::seconds_, "seconds", PinType::Number, new NumberInput(this, 0, 5000, &(t)), (Value *)(&t));
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    float t;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::seconds_])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::seconds_])->update();
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new RandomSource(t);
            Value source = (StartableSource *)(fs);
            input->node->trigger(source, input);
        }
    }
};

class WaveformNode : public EditorNode
{
public:
    enum InputKeys
    {
        function
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
        F *new_wave;
        switch (c->getSelectedId())
        {
        case Operations::sine:
            new_wave = new Sine(std::vector<F **>({&func}));
            break;
        case Operations::square:
            new_wave = new Square(std::vector<F **>({&func}));
            break;
        case Operations::sawtooth:
            new_wave = new Sawtooth(std::vector<F **>({&func}));
            break;
        case Operations::triangle:
            new_wave = new Triangle(std::vector<F **>({&func}));
            break;
        }
        waveform = new_wave;
    };
    WaveformNode()
    {
        func = nullptr;
        header = NodeNames::WaveformNode;
        type_id = (int)NodeTypes::Waveform;
        waveform = new Sine(std::vector<F **>({&func}));
        registerInput(InputKeys::function, "", PinType::Function);
        registerOutput(OutputKeys::wave_out, "wave", PinType::Function);
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
    F *waveform;

private:
    juce::ComboBox *c;
    F *func;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::function])
        {
            auto f = std::any_cast<F *>(data);
            func = f;
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::wave_out]))
        {
            Value source = waveform;
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
        header = NodeNames::AudioMathNode;
        type_id = (int)NodeTypes::AudioMath;
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

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
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
                    Value source = (StartableSource *)(s);
                    input->node->trigger(source, input);
                }
            }
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
        header = NodeNames::NumberMathNode;
        type_id = (int)NodeTypes::NumberMath;
        registerInput(InputKeys::number_1, "number", PinType::Number, new NumberInput(this, 0, 5000, &(val1)), (Value *)&(val1));
        registerInput(InputKeys::number_2, "number", PinType::Number, new NumberInput(this, 0, 5000, &(val2)), (Value *)&(val2));
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
            Value source = state->operation(val1, val2);
            input->node->trigger(source, input);
        }
    };

private:
    juce::ComboBox *c;
    MathAudioSource::State *state;

    float val1 = 0;
    float val2 = 0;
    float res;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin != nullptr)
        {
            if (float f = std::any_cast<float>(data))
            {
                if (pin == inputs[InputKeys::number_1])
                {
                    val1 = f;
                    ((NumberInput *)input_components[InputKeys::number_1])->update();
                }
                else if (pin == inputs[InputKeys::number_2])
                {
                    val2 = f;
                    ((NumberInput *)input_components[InputKeys::number_2])->update();
                }
            }
        }

        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::number_out]))
        {
            Value source = state->operation(val1, val2);
            input->node->trigger(source, input);
        }
    }
};

class ConcatenateNode : public EditorNode
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
    ConcatenateNode()
    {
        s1 = nullptr;
        s2 = nullptr;
        header = NodeNames::Concatenate;
        type_id = (int)NodeTypes::Concatenate;
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

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
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
                    Value source = (StartableSource *)(s);
                    input->node->trigger(source, input);
                }
            }
        }
    }
};

class FunctionMathNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        f,
        g
    };
    enum OutputKeys
    {
        h
    };
    FunctionMathNode()
    {
        val1 = nullptr;
        val2 = nullptr;
        states[Operations::add] = new Add(std::vector<F **>({&val1, &val2}));
        states[Operations::divide] = new Divide(std::vector<F **>({&val1, &val2}));
        states[Operations::substract] = new Substract(std::vector<F **>({&val1, &val2}));
        states[Operations::multiply] = new Multiply(std::vector<F **>({&val1, &val2}));
        c = nullptr;
        header = NodeNames::FunctionMathNode;
        type_id = (int)NodeTypes::FunctionMath;
        registerInput(InputKeys::f, "f", PinType::Function);
        registerInput(InputKeys::g, "g", PinType::Function);
        registerOutput(OutputKeys::h, "h", PinType::Function);
    };
    ~FunctionMathNode()
    {
    }
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
            result = states[Operations::add];
            break;
        case Operations::substract:
            result = states[Operations::substract];
            break;
        case Operations::multiply:
            result = states[Operations::multiply];
            break;
        case Operations::divide:
            result = states[Operations::divide];
            break;
        }
    };

    void valueChanged() override
    {
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::h]))
        {
            Value source = result;
            input->node->trigger(source, input);
        }
    };

private:
    std::unordered_map<Operations, F *> states;
    juce::ComboBox *c;
    F *result;

    F *val1;
    F *val2;
    float res;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin != nullptr)
        {
            if (F *f = std::any_cast<F *>(data))
            {
                if (pin == inputs[InputKeys::f])
                {
                    val1 = f;
                }
                else if (pin == inputs[InputKeys::g])
                {
                    val2 = f;
                }
            }
        }

        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::h]))
        {
            Value source = result;
            input->node->trigger(source, input);
        }
    }
};

class ConstFunctionNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        number
    };
    enum OutputKeys
    {
        func
    };
    ConstFunctionNode()
    {
        header = NodeNames::ConstNode;
        type_id = (int)NodeTypes::Const;
        t = 0;
        fs = new Const(t);
        registerOutput(OutputKeys::func, "number", PinType::Function);
        registerInput(InputKeys::number, "", PinType::Number, new NumberInput(this, 0, 5000, &(t)), (Value *)&(t));
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    float t;
    F *fs;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::number])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::number])->update();
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::func]))
        {

            Value source = (F *)(fs);
            input->node->trigger(source, input);
        }
    }
};

class OscillatorNode : public EditorNode, NumberInput::Listener
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

    OscillatorNode() : EditorNode()
    {
        header = NodeNames::Oscillator;
        type_id = (int)NodeTypes::Oscillator;
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::osc_, "wave", PinType::Function);
        registerInput(InputKeys::frequency_, "frequency", PinType::Number, new NumberInput(this, 0, 5000, &(frequency)), (Value *)(&frequency));
        registerInput(InputKeys::phase_, "phase", PinType::Number, new NumberInput(this, 0, 5000, &(phase)), (Value *)(&phase));
        registerInput(InputKeys::seconds_, "seconds", PinType::Number, new NumberInput(this, 0, 5000, &(t)), (Value *)(&t));
    };

    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    float frequency = 440;
    float phase = 0;
    float t = 1;
    F *wave = nullptr;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::frequency_])
        {
            frequency = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::frequency_])->update();
        }
        if (pin == inputs[InputKeys::seconds_])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::seconds_])->update();
        }
        if (pin == inputs[InputKeys::osc_])
        {
            wave = std::any_cast<F *>(data);
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            auto fs = new Osc(t, frequency, phase, &wave);
            Value source = (StartableSource *)(fs);
            input->node->trigger(source, input);
        }
    }
};

class NoundTypesFactory : public TypesRecoverFactory
{
public:
    NoundTypesFactory()
    {
        factories[NodeTypes::Output] = new NodeFactory<OutputNode>;
        factories[NodeTypes::Reverb] = new NodeFactory<ReverbNode>;
        factories[NodeTypes::Waveform] = new NodeFactory<WaveformNode>;
        factories[NodeTypes::FileReader] = new NodeFactory<FileReaderNode>;
        factories[NodeTypes::AudioMath] = new NodeFactory<AudioMathNode>;
        factories[NodeTypes::NumberMath] = new NodeFactory<NumberMathNode>;
        factories[NodeTypes::Concatenate] = new NodeFactory<ConcatenateNode>;
        factories[NodeTypes::FunctionMath] = new NodeFactory<FunctionMathNode>;
        factories[NodeTypes::Const] = new NodeFactory<ConstFunctionNode>;
        factories[NodeTypes::Random] = new NodeFactory<RandomNode>;
        factories[NodeTypes::Oscillator] = new NodeFactory<OscillatorNode>;
    }

    EditorNode *getNode(int type_id) override
    {
        return factories[(NodeTypes)type_id]->create();
    }
    std::map<NodeTypes, AbstractNodeFactory *> factories;

    ~NoundTypesFactory()
    {
        for (auto &[_, f] : factories)
        {
            delete f;
        }
        factories.clear();
    }
};