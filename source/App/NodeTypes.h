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
            for (auto &input : graph->getInputsOfOutput(&p))
            {
                input->node->triggerAsync(data, input);
            }
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
    std::string name;
    juce::URL *currentAudioFile;
    FileInput *internal;
    FileSource filesource;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(&outputs[OutputKeys::audio_]))
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
        for (auto &input : graph->getInputsOfOutput(&outputs[OutputKeys::trigger_out]))
        {
            input->node->triggerAsync(source, input);
        }
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

        if (slider == &damping)
        {
            p.damping = val;
        }
        else if (slider == &dryLevel)
        {
            p.dryLevel = val;
        }
        else if (slider == &freezeMode)
        {
            p.freezeMode = val;
        }
        else if (slider == &roomSize)
        {
            p.roomSize = val;
        }
        else if (slider == &wetLevel)
        {
            p.wetLevel = val;
        }
        else if (slider == &width)
        {
            p.width = val;
        }
    }

    ReverbNode() : EditorNode()
    {
        int i = 0;
        int h = ThemeProvider::getCurrentTheme()->nodeTextHeight;
        for (auto &c : cs)
        {
            labels[i]->setSize(100, h * 1.5);
            labels[i]->setFont(h);
            internal.addComponent(labels[i]);
            c->setRange(0, 1);         // [1]
            c->setTextValueSuffix(""); // [2]
            c->addListener(this);
            c->setValue(0.5);
            sliderValueChanged(c);
            c->setSize(1000, h);
            internal.addComponent(c);
            i++;
        }
        internal.setSize(1000, h * (cs.size()) * 2.5);

        header = "Reverb";
        registerInput(InputKeys::audio_in, "audio", PinType::Signal);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Signal);
    };

    ~ReverbNode()
    {
        for (auto &n : labels)
            delete n;
        labels.clear();
    }

    juce::Component *getInternal() override
    {
        return &internal;
    }

private:
    juce::Reverb::Parameters p;
    juce::Slider width, dryLevel, damping, freezeMode, roomSize, wetLevel;
    std::vector<juce::Slider *> cs = {&width, &dryLevel, &damping, &freezeMode, &roomSize, &wetLevel};
    std::vector<juce::Label *> labels = {new juce::Label({}, "width"), new juce::Label({}, "dryLevel"), new juce::Label({}, "damping"), new juce::Label({}, "freezeMode"), new juce::Label({}, "roomSize"), new juce::Label({}, "wetLevel")};
    Vertical internal;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        StartableSource *input_source = std::any_cast<StartableSource *>(data);
        for (auto &input : graph->getInputsOfOutput(&outputs[OutputKeys::audio_out]))
        {
            auto fs = new ReverbSource();
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
        registerInput(InputKeys::start, "start", PinType::Control);

        registerOutput(OutputKeys::audio_out, "audio", PinType::Signal);
    };
    juce::Component *getInternal() override
    {
        return nullptr;
    }

private:
    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(&outputs[OutputKeys::audio_out]))
        {
            auto fs = new RandomSource();
            Data source = (StartableSource *)(fs);
            input->node->triggerAsync(source, input);
        }
    }
};

class SineNode : public EditorNode, public juce::Slider::Listener
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
    void sliderValueChanged(juce::Slider *slider) override
    {
        double val = slider->getValue();
        frequency = val;
    }
    SineNode()
    {
        header = "Sine";
        registerInput(InputKeys::start, "start", PinType::Control);
        registerOutput(OutputKeys::audio_out, "audio", PinType::Signal);

        int h = ThemeProvider::getCurrentTheme()->nodeTextHeight;
        auto l = new juce::Label({}, "frequency");
        l->setSize(1000, h * 1.5);
        l->setFont(h);
        internal.addComponent(l);

        auto s = new juce::Slider();
        s->setSize(1000, h * 1.5);
        s->setRange(0.01, 1000);   // [1]
        s->setTextValueSuffix(""); // [2]
        s->addListener(this);
        s->setValue(440);
        internal.addComponent(s);

        internal.setSize(1000, h * 3);
    };
    juce::Component *getInternal() override
    {
        return &internal;
    }

private:
    Vertical internal;
    float frequency = 440;

    void trigger(Data &data, [[maybe_unused]] Input *pin) override
    {
        for (auto &input : graph->getInputsOfOutput(&outputs[OutputKeys::audio_out]))
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
            if (pin == &inputs[InputKeys::audio_1])
            {
                s1 = f;
            }
            else if (pin == &inputs[InputKeys::audio_2])
            {
                s2 = f;
            }
            if (s1 != nullptr && s2 != nullptr)
            {
                for (auto &input : graph->getInputsOfOutput(&outputs[OutputKeys::audio_out]))
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
        registerInput(InputKeys::audio_in, "audio", PinType::Control);
        registerOutput(OutputKeys::number_out, "number", PinType::Signal);
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