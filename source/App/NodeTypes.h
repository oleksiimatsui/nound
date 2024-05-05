#pragma once
#include "NodeGraph.h"
#include "EditorNode.h"
#include <JuceHeader.h>
#include "vector"
#include "Theme.h"
#include "Components.h"
#include "Sources.h"

#include "NodeTypesRegistry.h"

class AbstractNodeCreateCommand
{
public:
    virtual EditorNode *execute() = 0;
};

template <class T>
class NodeCreateCommand : public AbstractNodeCreateCommand
{
public:
    EditorNode *execute() override
    {
        return new T;
    }
};

// audio

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

class FileReaderNode : public EditorNode, public FileInput::Listener
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
        registerInternal(new FileInput(new StringRef(name), this));
        header = NodeNames::FileReader;
        type_id = (int)NodeTypes::FileReader;
        outputs[OutputKeys::audio_] = new Output(OutputKeys::audio_, "audio", PinType::Audio, this);
    };

    ~FileReaderNode()
    {
    }

    void fileChanged() override
    {
        auto fs = new FileSource();
        fs->setFile(name);

        source.reset(fs);
    };

private:
    std::string name;
    juce::URL *currentAudioFile;
    std::unique_ptr<FileSource> source;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::predecessor])
        {
        }
        auto connection_inputs = graph->getInputsOfOutput(outputs[OutputKeys::audio_]);
        for (int i = 0; i < connection_inputs.size(); i++)
        {
            auto input = connection_inputs[i];
            input->node->trigger((Value)((StartableSource *)source.get()), input);
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
        registerInput(InputKeys::width, "width", PinType::Number, new NumberInput(this, 0, 1, new FloatRef(p.width)));
        registerInput(InputKeys::damping, "damping", PinType::Number, new NumberInput(this, 0, 1, new FloatRef(p.damping)));
        registerInput(InputKeys::dryLevel, "dryLevel", PinType::Number, new NumberInput(this, 0, 1, new FloatRef(p.dryLevel)));
        registerInput(InputKeys::freezeMode, "freezeMode", PinType::Number, new NumberInput(this, 0, 1, new FloatRef(p.freezeMode)));
        registerInput(InputKeys::roomSize, "roomSize", PinType::Number, new NumberInput(this, 0, 1, new FloatRef(p.roomSize)));
        registerInput(InputKeys::wetLevel, "wetLevel", PinType::Number, new NumberInput(this, 0, 1, new FloatRef(p.wetLevel)));

        valueChanged();
    };
    ~ReverbNode()
    {
        for (auto &n : results)
            delete n;
        results.clear();
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

class AudioMathNode : public EditorNode, juce::ComboBox::Listener
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
        header = NodeNames::AudioMathNode;
        type_id = (int)NodeTypes::AudioMath;
        registerInput(InputKeys::audio_1, "audio", PinType::Audio);
        registerInput(InputKeys::audio_2, "audio", PinType::Audio);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInternal(new Selector(new IntRef(selected), this, std::vector<std::string>({"Add", "Subtract", "Multiply", "Divide"}), 1));
    };
    enum Operations
    {
        add = 1,
        substract,
        multiply,
        divide
    };

    void comboBoxChanged(juce::ComboBox *c) override
    {
        switch (selected)
        {
        case Operations::add:
            state.reset(new MathAudioSource::Add());
            break;
        case Operations::substract:
            state.reset(new MathAudioSource::Substract());
            break;
        case Operations::multiply:
            state.reset(new MathAudioSource::Multiply());
            break;
        case Operations::divide:
            state.reset(new MathAudioSource::Divide());
            break;
        }
    };

private:
    int selected;
    std::unique_ptr<MathAudioSource::State> state;
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
                    s->state = &state;
                    Value source = (StartableSource *)(s);
                    input->node->trigger(source, input);
                }
            }
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
        registerInput(InputKeys::frequency_, "frequency", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(frequency)));
        registerInput(InputKeys::phase_, "phase", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(phase)));
        registerInput(InputKeys::seconds_, "seconds", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(t)));
    };

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
            if (wave == nullptr)
            {
                wave = new Sine(std::vector<F **>({}));
            }
            auto fs = new Osc(t, frequency, phase, &wave);
            Value source = (StartableSource *)(fs);
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

class RepeatNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        audio_,
        seconds_,
    };
    enum OutputKeys
    {
        audio_out
    };
    RepeatNode() : EditorNode()
    {
        t = 1;
        audio = nullptr;
        header = NodeNames::RepeatNode;
        type_id = (int)NodeTypes::RepeatNode;
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::audio_, "audio", PinType::Audio);
        registerInput(InputKeys::seconds_, "seconds", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(t)));
    };

private:
    float t = 1;
    StartableSource *audio;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == inputs[InputKeys::seconds_])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::seconds_])->update();
        }
        if (pin == inputs[InputKeys::audio_])
        {
            audio = std::any_cast<StartableSource *>(data);
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::audio_out]))
        {
            if (audio == nullptr)
            {
                return;
            }
            auto res = new RepeatSource(t);
            res->source = audio;
            Value source = (StartableSource *)(res);
            input->node->trigger(source, input);
        }
    }
};

// function

class FunctionMathNode : public EditorNode, juce::ComboBox::Listener
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
        header = NodeNames::FunctionMathNode;
        type_id = (int)NodeTypes::FunctionMath;
        registerInput(InputKeys::f, "f", PinType::Function);
        registerInput(InputKeys::g, "g", PinType::Function);
        registerOutput(OutputKeys::h, "h", PinType::Function);
        registerInternal(new Selector(new IntRef(selected_state), this, std::vector<std::string>({"Add", "Substract", "Multiply", "Divide"}), 1));
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

    void comboBoxChanged(juce::ComboBox *c) override
    {
        switch (selected_state)
        {
        case Operations::add:
            result.reset(new Add(std::vector<F **>({&val1, &val2})));
            break;
        case Operations::substract:
            result.reset(new Substract(std::vector<F **>({&val1, &val2})));
            break;
        case Operations::multiply:
            result.reset(new Multiply(std::vector<F **>({&val1, &val2})));
            break;
        case Operations::divide:
            result.reset(new Divide(std::vector<F **>({&val1, &val2})));
            break;
        }
    };

private:
    std::unordered_map<Operations, F *> states;
    int selected_state;
    std::unique_ptr<F> result;
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
            Value source = result.get();
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
        registerInput(InputKeys::number, "", PinType::Number, new NumberInput(this, -50000, 50000, new FloatRef(t)));
    };

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

class WaveformNode : public EditorNode, juce::ComboBox::Listener
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
    void comboBoxChanged(juce::ComboBox *c) override
    {
        switch (c->getSelectedId())
        {
        case Operations::sine:
            waveform.reset(new Sine(std::vector<F **>({&func})));
            break;
        case Operations::square:
            waveform.reset(new Square(std::vector<F **>({&func})));
            break;
        case Operations::sawtooth:
            waveform.reset(new Sawtooth(std::vector<F **>({&func})));
            break;
        case Operations::triangle:
            waveform.reset(new Triangle(std::vector<F **>({&func})));
            break;
        }
    };
    WaveformNode()
    {
        func = nullptr;
        header = NodeNames::WaveformNode;
        type_id = (int)NodeTypes::Waveform;
        waveform.reset(new Sine(std::vector<F **>({&func})));
        selected_wave = 1;
        registerInput(InputKeys::function, "", PinType::Function);
        registerOutput(OutputKeys::wave_out, "wave", PinType::Function);
        registerInternal(new Selector(new IntRef(selected_wave), this, std::vector<std::string>({"Sine", "Square", "Sawtooth", "Triangle"}), 1));
    };
    std::unique_ptr<F> waveform;

private:
    int selected_wave;
    F *func;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        delete func;
        func = nullptr;
        if (pin == inputs[InputKeys::function])
        {
            auto f = std::any_cast<F *>(data);
            func = f;
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::wave_out]))
        {
            Value source = waveform.get();
            input->node->trigger(source, input);
        }
    }
};

class RandomNode : public EditorNode
{
public:
    enum InputKeys
    {

    };
    enum OutputKeys
    {
        random
    };
    RandomNode()
    {
        randomF.reset(new Random());
        header = NodeNames::RandomNode;
        type_id = (int)NodeTypes::Random;
        registerOutput(OutputKeys::random, "random", PinType::Function);
    };

private:
    std::unique_ptr<F> randomF;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::random]))
        {
            Value source = randomF.get();
            input->node->trigger(source, input);
        }
    }
};

class LineNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        start_,
        end_,
        function_in
    };
    enum OutputKeys
    {
        line_
    };
    LineNode()
    {
        f = nullptr;
        start = 0;
        end = 0;
        line.reset(new Line(start, end, std::vector<F **>({&f})));
        header = NodeNames::LineNode;
        type_id = (int)NodeTypes::LineNode;
        registerOutput(OutputKeys::line_, "line", PinType::Function);
        registerInput(InputKeys::start_, "start", PinType::Number, new NumberInput(this, -50000, 50000, new FloatRef(start)));
        registerInput(InputKeys::end_, "end", PinType::Number, new NumberInput(this, -50000, 50000, new FloatRef(end)));
        registerInput(InputKeys::function_in, "", PinType::Function);
    };

private:
    std::unique_ptr<F> line;
    float start;
    float end;
    F *f;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        delete f;
        f = nullptr;
        if (pin == inputs[InputKeys::function_in])
        {
            auto f_ = std::any_cast<F *>(data);
            f = f_;
        }
        if (pin == inputs[InputKeys::start_])
        {
            start = std::any_cast<float>(data);
        }
        if (pin == inputs[InputKeys::start_])
        {
            end = std::any_cast<float>(data);
        }
        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::line_]))
        {
            Value source = line.get();
            input->node->trigger(source, input);
        }
    }
};

class ConcatenateFNode : public EditorNode
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
    ConcatenateFNode()
    {
        val1 = nullptr;
        val2 = nullptr;
        header = NodeNames::ConcatenateFunction;
        type_id = (int)NodeTypes::ConcatenateFunction;
        registerInput(InputKeys::f, "f", PinType::Function);
        registerInput(InputKeys::g, "g", PinType::Function);
        registerOutput(OutputKeys::h, "h", PinType::Function);
        result.reset(new Concatenate(std::vector<F **>({&val1, &val2})));
    };

private:
    std::unique_ptr<F> result;
    F *val1;
    F *val2;

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
            if (result.get() == nullptr)
                return;
            Value source = result.get();
            input->node->trigger(source, input);
        }
    }
};

// number

class NumberMathNode : public EditorNode,
                       juce::ComboBox::Listener,
                       NumberInput::Listener
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
        header = NodeNames::NumberMathNode;
        type_id = (int)NodeTypes::NumberMath;
        registerInput(InputKeys::number_1, "number", PinType::Number, new NumberInput(this, -50000, 50000, new FloatRef(val1)));
        registerInput(InputKeys::number_2, "number", PinType::Number, new NumberInput(this, -50000, 50000, new FloatRef(val2)));
        registerOutput(OutputKeys::number_out, "number", PinType::Number);
        registerInternal(new Selector(new IntRef(selected_state), this, std::vector<std::string>({"Add", "Substract", "Multiply", "Divide"}), 1));
    }
    enum Operations
    {
        add = 1,
        substract,
        multiply,
        divide
    };

    void comboBoxChanged(juce::ComboBox *c) override
    {
        switch (selected_state)
        {
        case Operations::add:
            state.reset(new MathAudioSource::Add());
            break;
        case Operations::substract:
            state.reset(new MathAudioSource::Substract());
            break;
        case Operations::multiply:
            state.reset(new MathAudioSource::Multiply());
            break;
        case Operations::divide:
            state.reset(new MathAudioSource::Divide());
            break;
        }
    };
    void valueChanged() override
    {
    }

private:
    juce::ComboBox *c;
    std::unique_ptr<MathAudioSource::State> state;
    int selected_state;
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
                    input_components[InputKeys::number_1]->update();
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
            Value source = (float)state->operation(val1, val2);
            input->node->trigger(source, input);
        }
    }
};

class NumberNode : public EditorNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        number_
    };
    enum OutputKeys
    {
        number_out
    };
    void valueChanged() override
    {

        for (auto &input : graph->getInputsOfOutput(outputs[OutputKeys::number_out]))
        {
            Value source = value;
            input->node->trigger(source, input);
        }
    }
    NumberNode()
    {
        header = NodeNames::NumberNode;
        type_id = (int)NodeTypes::NumberNode;
        registerInput(InputKeys::number_, "number", PinType::Number, new NumberInput(this, -50000, 50000, new FloatRef(value)));
        registerOutput(OutputKeys::number_out, "number", PinType::Number);
    }

private:
    float value = 0;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin != nullptr)
        {
            if (float f = std::any_cast<float>(data))
            {
                if (pin == inputs[InputKeys::number_])
                {
                    value = f;
                    input_components[InputKeys::number_]->update();
                }
            }
        }
        valueChanged();
    }
};