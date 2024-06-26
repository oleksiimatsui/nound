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

class AudioNode : public EditorNode
{
public:
    AudioNode(int _output_id) : EditorNode(), output_id(_output_id){

                                              };
    ~AudioNode()
    {
        for (auto &s : sources)
        {
            delete s;
            s = nullptr;
        }
        sources.clear();
    }

protected:
    void clearSources()
    {
        for (auto &s : sources)
        {
            delete s;
            s = nullptr;
        }
        sources.clear();
    }
    void passSources(std::function<PositionableSource *()> func)
    {
        auto connection_inputs = graph->getInputsOfOutput(outputs[output_id]);
        for (int i = 0; i < connection_inputs.size(); i++)
        {
            auto input = connection_inputs[i];
            if (i == sources.size())
            {
                sources.push_back(func());
            }
            input->node->trigger((Value)((PositionableSource *)sources[i]), input);
        }
    }
    std::vector<PositionableSource *> sources;
    int output_id;
};

class OutputNode : public EditorNode
{
public:
    enum InputKeys
    {
        audio_
    };
    OutputNode() : EditorNode()
    {
        result = nullptr;
        header = NodeNames::OutputNode;
        type_id = (int)NodeTypes::Output;
        inputs[InputKeys::audio_] = new Input(InputKeys::audio_, "audio", PinType::Audio, this);
    };
    // SoundOutputSource source;
    PositionableSource *result;

private:
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        result = nullptr;
        if (pin == inputs[InputKeys::audio_])
        {
            result = std::any_cast<PositionableSource *>(data);
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
    };
    enum OutputKeys
    {
        audio_,
        length_out
    };
    FileReaderNode() : EditorNode()
    {
        currentAudioFile = nullptr;
        registerInternal(new FileInput(new StringRef(name), this));
        header = NodeNames::FileReader;
        type_id = (int)NodeTypes::FileReader;
        outputs[OutputKeys::audio_] = new Output(OutputKeys::audio_, "audio", PinType::Audio, this);
        registerOutput(OutputKeys::length_out, "seconds", PinType::Number);
        t = 0;
    };

    ~FileReaderNode()
    {
    }

    void fileChanged() override
    {
        for (auto &s : sources)
        {
            s->setFile(name);
        }
        if (sources.size() > 0)
            t = sources[0]->getLengthInSeconds();
    };

private:
    float t;
    std::string name;
    juce::URL *currentAudioFile;
    std::vector<std::unique_ptr<FileSource>> sources;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (name == "")
            return;
        auto connection_inputs = graph->getInputsOfOutput(outputs[OutputKeys::audio_]);
        for (int i = 0; i < connection_inputs.size(); i++)
        {
            auto input = connection_inputs[i];
            if (i == sources.size())
            {
                std::unique_ptr<FileSource> f;
                f.reset(new FileSource());
                sources.push_back(std::move(f));
            }
            bool r = sources[i]->setFile(name);
            if (r)
            {
                input->node->trigger((Value)((PositionableSource *)sources[i].get()), input);
                if (sources.size() > 0)
                    t = sources[0]->getLengthInSeconds();
            }
        }

        connection_inputs = graph->getInputsOfOutput(outputs[OutputKeys::length_out]);
        for (int i = 0; i < connection_inputs.size(); i++)
        {
            auto input = connection_inputs[i];
            input->node->trigger((Value)((float)t), input);
        }
    }
};

class ReverbNode : public AudioNode, public NumberInput::Listener
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
        for (auto &r : sources)
        {
            ((ReverbSource *)r)->r->setParameters(p);
        }
    }
    ReverbNode() : AudioNode(0)
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

private:
    juce::Reverb::Parameters p;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == nullptr)
            return;
        if (sources.size() != 0)
        {
            clearSources();
        }
        PositionableSource *input_source = std::any_cast<PositionableSource *>(data);
        passSources([&]() -> PositionableSource *
                    {
                    auto fs = new ReverbSource();
                    fs->setSource(input_source);
                    fs->r->setParameters(p); 
                    return fs; });
    }
};

class AudioMathNode : public AudioNode, juce::ComboBox::Listener
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

    AudioMathNode() : AudioNode(0)
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
    PositionableSource *s1;
    PositionableSource *s2;
    int counter = 0;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == nullptr)
            return;
        PositionableSource *f = std::any_cast<PositionableSource *>(data);
        if (counter == getConnectionsNumber())
        {
            counter = 0;
        }
        clearSources();
        if (pin == inputs[InputKeys::audio_1])
        {
            s1 = f;
            counter++;
        }
        else if (pin == inputs[InputKeys::audio_2])
        {
            s2 = f;
            counter++;
        }

        if (counter == getConnectionsNumber())
            if (s1 != nullptr && s2 != nullptr)
            {
                passSources([&]() -> PositionableSource *
                            {
                            auto s = new MathAudioSource();
                            s->s1 = s1;
                            s->s2 = s2;
                            s->state = &state;
                            return s; });
            }
    }
};

class OscillatorNode : public AudioNode, NumberInput::Listener
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
        audio_out,
        length_out
    };

    OscillatorNode() : AudioNode(0)
    {
        header = NodeNames::Oscillator;
        type_id = (int)NodeTypes::Oscillator;
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerOutput(OutputKeys::length_out, "seconds", PinType::Number);

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
        if (sources.size() != 0)
        {
            clearSources();
        }
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
        else
        {
            wave = nullptr;
        }

        auto connection_inputs = graph->getInputsOfOutput(outputs[OutputKeys::length_out]);
        for (int i = 0; i < connection_inputs.size(); i++)
        {
            auto input = connection_inputs[i];
            input->node->trigger((Value)((float)t), input);
        }

        passSources([&]() -> PositionableSource *
                    {
                    if (wave == nullptr)
                    {
                        wave = new Sine(std::vector<F **>({}));
                    }
                    auto fs = new Osc(t, frequency, phase, &wave);
                    return fs; });
    }
};

class ConcatenateNode : public AudioNode
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
    ConcatenateNode() : AudioNode(0)
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
    PositionableSource *s1;
    PositionableSource *s2;
    int counter = 0;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == nullptr)
            return;
        if (counter == getConnectionsNumber())
        {
            counter = 0;
        }
        if (PositionableSource *f = std::any_cast<PositionableSource *>(data))
        {
            if (sources.size() != 0)
            {
                clearSources();
            }
            if (pin == inputs[InputKeys::audio_1])
            {
                s1 = f;
                counter++;
            }
            else if (pin == inputs[InputKeys::audio_2])
            {
                s2 = f;
                counter++;
            }

            if (counter == getConnectionsNumber())

                if (s1 != nullptr && s2 != nullptr)
                {
                    passSources([&]() -> PositionableSource *
                                {
                            auto s = new ConcatenationSource();
                            s->setSources(std::vector<PositionableSource *>({s1, s2}));
                            return s; });
                }
        }
    }
};

class RepeatNode : public AudioNode, NumberInput::Listener
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
    RepeatNode() : AudioNode(0)
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
    PositionableSource *audio;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == nullptr)
            return;
        if (sources.size() != 0)
        {
            clearSources();
        }
        if (pin == inputs[InputKeys::seconds_])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::seconds_])->update();
        }
        if (pin == inputs[InputKeys::audio_])
        {
            audio = std::any_cast<PositionableSource *>(data);
        }

        if (audio == nullptr)
        {
            return;
        }
        passSources([&]() -> PositionableSource *
                    {
                            auto res = new RepeatSource(t);
                            res->source = audio;
                            return res; });
    }
};

class TrimNode : public AudioNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        audio_,
        start_,
        duration_,
    };
    enum OutputKeys
    {
        audio_out
    };
    TrimNode() : AudioNode(0)
    {
        t = 1;
        start = 0;
        audio = nullptr;
        header = NodeNames::TrimNode;
        type_id = (int)NodeTypes::TrimNode;
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::audio_, "audio", PinType::Audio);
        registerInput(InputKeys::start_, "start at (seconds)", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(start)));
        registerInput(InputKeys::duration_, "duration (seconds)", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(t)));
    };

private:
    float t;
    float start;
    PositionableSource *audio;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == nullptr)
            return;
        if (sources.size() != 0)
        {
            clearSources();
        }
        if (pin == inputs[InputKeys::start_])
        {
            start = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::start_])->update();
        }
        if (pin == inputs[InputKeys::duration_])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::duration_])->update();
        }
        if (pin == inputs[InputKeys::audio_])
        {
            audio = std::any_cast<PositionableSource *>(data);
        }

        if (audio == nullptr)
        {
            return;
        }

        passSources([&]() -> PositionableSource *
                    {
                            auto res = new TrimSource(start, t);
            res->source = audio;
                            return res; });
    }
};

class ResamplingNode : public AudioNode, NumberInput::Listener
{
public:
    enum InputKeys
    {
        audio_,
        coefficient_,
    };
    enum OutputKeys
    {
        audio_out
    };
    ResamplingNode() : AudioNode(0)
    {
        coefficient = 1;
        audio = nullptr;
        header = NodeNames::ResamplingNode;
        type_id = (int)NodeTypes::ResamplingNode;
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::audio_, "audio", PinType::Audio);
        registerInput(InputKeys::coefficient_, "coefficient", PinType::Number, new NumberInput(this, 0.01, 100, new FloatRef(coefficient)));
    };
    void valueChanged() override
    {
        for (auto &s : sources)
        {
            ((ResamplingAudioSource *)s)->updateCoefficient();
        }
    }

private:
    float coefficient;
    PositionableSource *audio;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == nullptr)
            return;

        clearSources();

        if (pin == inputs[InputKeys::coefficient_])
        {
            coefficient = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::coefficient_])->update();
        }
        if (pin == inputs[InputKeys::audio_])
        {
            audio = std::any_cast<PositionableSource *>(data);
        }

        if (audio == nullptr)
        {
            return;
        }

        passSources([&]() -> PositionableSource *
                    {
                            auto res = new ResamplingAudioSource(audio,coefficient);
                            return res; });
    }
};

class BandPassNode : public AudioNode, public NumberInput::Listener
{
public:
    enum InputKeys
    {
        audio_in,
        f0_,
        f1_,
    };
    enum OutputKeys
    {
        audio_out
    };
    void valueChanged() override
    {
        for (auto &r : sources)
        {
            ((FilterSource *)r)->update();
        }
    }
    BandPassNode() : AudioNode(0)
    {
        f0 = 400;
        f1 = 480;
        header = NodeNames::FilterNode;
        type_id = (int)NodeTypes::FilterNode;
        registerInput(InputKeys::audio_in, "audio", PinType::Audio);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Audio);
        registerInput(InputKeys::f0_, "bottom frequency", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(f0)));
        registerInput(InputKeys::f1_, "top frequency", PinType::Number, new NumberInput(this, 0, 5000, new FloatRef(f1)));
        valueChanged();
    };

private:
    juce::IIRCoefficients coefficients;
    float f0, f1;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (pin == nullptr)
            return;

        clearSources();

        PositionableSource *input_source = std::any_cast<PositionableSource *>(data);
        passSources([&]() -> PositionableSource *
                    {
                    auto fs = new FilterSource(f0,f1);
                    fs->setSource(input_source);
                    return fs; });
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
    int counter = 0;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (counter == getConnectionsNumber())
        {
            counter = 0;
        }
        if (pin != nullptr)
        {
            if (F *f = std::any_cast<F *>(data))
            {
                if (pin == inputs[InputKeys::f])
                {
                    val1 = f;
                    counter++;
                }
                else if (pin == inputs[InputKeys::g])
                {
                    val2 = f;
                    counter++;
                }
            }
        }
        if (counter == getConnectionsNumber())
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
        int counter = 0;
        if (pin == inputs[InputKeys::number])
        {
            t = std::any_cast<float>(data);
            ((NumberInput *)input_components[InputKeys::number])->update();
            counter++;
        }
        if (counter == getConnectionsNumber())
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
    int counter = 0;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (counter == getConnectionsNumber())
        {
            counter = 0;
        }
        if (pin == inputs[InputKeys::function])
        {
            auto f = std::any_cast<F *>(data);
            func = f;
            counter++;
        }
        if (counter == getConnectionsNumber())
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
    ~RandomNode() override
    {
        //    delete randomF.get();
        randomF.reset(nullptr);
    }

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
    int counter = 0;

    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (counter == getConnectionsNumber())
        {
            counter = 0;
        }
        if (pin == inputs[InputKeys::function_in])
        {
            auto f_ = std::any_cast<F *>(data);
            f = f_;
            counter++;
        }
        if (pin == inputs[InputKeys::start_])
        {
            start = std::any_cast<float>(data);
            counter++;
        }
        if (pin == inputs[InputKeys::end_])
        {
            end = std::any_cast<float>(data);
            counter++;
        }
        if (counter == getConnectionsNumber())
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
    int counter = 0;
    void trigger(Value &data, [[maybe_unused]] Input *pin) override
    {
        if (counter == getConnectionsNumber())
        {
            counter = 0;
        }
        if (pin != nullptr)
        {
            if (F *f = std::any_cast<F *>(data))
            {
                if (pin == inputs[InputKeys::f])
                {
                    val1 = f;
                    counter++;
                }
                else if (pin == inputs[InputKeys::g])
                {
                    val2 = f;
                    counter++;
                }
            }
        }
        if (counter == getConnectionsNumber())
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
        int counter = 0;
        if (pin != nullptr)
        {
            if (float f = std::any_cast<float>(data))
            {
                if (pin == inputs[InputKeys::number_1])
                {
                    val1 = f;
                    input_components[InputKeys::number_1]->update();
                    counter++;
                }
                else if (pin == inputs[InputKeys::number_2])
                {
                    val2 = f;
                    ((NumberInput *)input_components[InputKeys::number_2])->update();
                    counter++;
                }
            }
        }
        if (counter == getConnectionsNumber())
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
        int counter = 0;
        if (pin != nullptr)
        {
            if (float f = std::any_cast<float>(data))
            {
                if (pin == inputs[InputKeys::number_])
                {
                    value = f;
                    input_components[InputKeys::number_]->update();
                    counter++;
                }
            }
        }
        if (counter == getConnectionsNumber())
            valueChanged();
    }
};