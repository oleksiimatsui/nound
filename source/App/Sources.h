#pragma once
#include <JuceHeader.h>

class StartableSource : public juce::AudioSource
{
public:
    StartableSource(){};
    virtual void Start() = 0;
    virtual void Stop() = 0;

private:
};

class ReverbSource : public StartableSource
{
public:
    ReverbSource()
    {
        source = nullptr;
        r = nullptr;
    }
    void setSource(StartableSource *s)
    {
        source = s;
        r = new juce::ReverbAudioSource(s, false);
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        r->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        r->releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        r->getNextAudioBlock(bufferToFill);
    };
    void Start() override
    {
        source->Start();
    };
    void Stop() override
    {
        source->Stop();
    };
    StartableSource *source;
    juce::ReverbAudioSource *r;
};

class SoundOutputSource : public StartableSource
{
public:
    SoundOutputSource()
    {
        source = nullptr;
    };

    void setSource(StartableSource *s)
    {
        source = s;
    };

    void setAudioChannels(int numInputChannels, int numOutputChannels, const juce::XmlElement *const xml = nullptr)
    {
        juce::String audioError;
        audioError = deviceManager.initialise(numInputChannels, numOutputChannels, xml, true);
        jassert(audioError.isEmpty());
        deviceManager.addAudioCallback(&audioSourcePlayer);
        audioSourcePlayer.setSource(this);
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        if (source == nullptr)
            return;
        source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        if (source == nullptr)
            return;

        source->releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        if (source == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        source->getNextAudioBlock(bufferToFill);
    };

    void Start() override
    {
        if (source == nullptr)
            return;

        setAudioChannels(0, 1);
        source->Start();
    }
    void Stop() override
    {
        if (source == nullptr)
            return;

        source->Stop();
        audioSourcePlayer.setSource(nullptr);
        deviceManager.removeAudioCallback(&audioSourcePlayer);
        deviceManager.closeAudioDevice();
    }

private:
    StartableSource *source;
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
};

class FileSource : public StartableSource
{
public:
    FileSource()
    {
    }
    void setFile(std::string filepath)
    {
        transportSource.stop();
        path = filepath;
        file = juce::File(filepath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        juce::AudioFormatReader *reader = formatManager.createReaderFor(file);
        jassert((reader != nullptr));
        juce::AudioFormatReaderSource *readerSource = new juce::AudioFormatReaderSource(reader, true);
        //   thread.startThread(juce::Thread::Priority::normal);
        transportSource.setSource(readerSource,
                                  0, nullptr,
                                  readerSource->getAudioFormatReader()->sampleRate);
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        transportSource.releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        if (transportSource.isPlaying())
        {
            transportSource.getNextAudioBlock(bufferToFill);
        }
        else
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
    };
    void Start() override
    {
        Stop();
        transportSource.setPosition(0);
        transportSource.start();
    }
    void Stop() override
    {
        transportSource.stop();
    }

    std::string path;
    juce::AudioTransportSource transportSource;
    juce::File file;
    //  juce::TimeSliceThread thread{"audio file preview"};
};

class RandomSource : public StartableSource
{
public:
    RandomSource()
    {
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        random.setSeed(0);
    }
    void releaseResources() override
    {
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            // Get a pointer to the start sample in the buffer for this audio output channel
            auto *buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

            // Fill the required number of samples with noise between -0.125 and +0.125
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                buffer[sample] = random.nextFloat() * 0.25f - 0.125f;
        }
    }
    void Start() override
    {
    }
    void Stop() override
    {
    }
    juce::Random random;
};

class SineSource : public StartableSource
{
public:
    SineSource(float &f, float &p) : frequency(f), phase(p), StartableSource()
    {
    }

    void prepareToPlay(int samplesPerBlockExpected, double _sampleRate) override
    {
        sampleRate = _sampleRate;
        period = 1.0f / sampleRate;
    }
    void releaseResources() override
    {
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            auto *buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                if (n >= std::numeric_limits<int>::max())
                {
                    n = 0;
                }
                auto currentSample = (float)std::sin(2.0f * juce::MathConstants<float>::pi * frequency * period * n + phase);
                n++;
                buffer[sample] = currentSample;
            }
        }
    }
    void Start() override
    {
    }
    void Stop() override
    {
    }
    float &frequency;
    float &phase;
    float period = 0.0;
    int n = 0;
    double sampleRate = 0;
};

class MathAudioSource : public StartableSource
{
public:
    class State
    {
    public:
        virtual float operation(float a, float b)
        {
            return 0;
        }
    };
    class Add : public State
    {
    public:
        float operation(float a, float b) override
        {
            return a + b;
        }
    };
    class Substract : public State
    {
    public:
        float operation(float a, float b) override
        {
            return a - b;
        }
    };
    class Multiply : public State
    {
    public:
        float operation(float a, float b) override
        {
            return a * b;
        }
    };
    class Divide : public State
    {
    public:
        float operation(float a, float b) override
        {
            return a / b;
        }
    };
    class StateHolder
    {
    public:
        virtual State *getState() = 0;
    };
    MathAudioSource()
    {
        s1 = nullptr;
        s2 = nullptr;
    }
    ~MathAudioSource()
    {
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        if (s1 == nullptr || s2 == nullptr)
            return;
        s2->prepareToPlay(samplesPerBlockExpected, sampleRate);
        s1->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        if (s1 == nullptr || s2 == nullptr)
            return;
        s1->releaseResources();
        s2->releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        if (s1 == nullptr || s2 == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        temp1.setSize(juce::jmax(1, bufferToFill.buffer->getNumChannels()),
                      bufferToFill.buffer->getNumSamples());
        temp2.setSize(juce::jmax(1, bufferToFill.buffer->getNumChannels()),
                      bufferToFill.buffer->getNumSamples());
        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            s1->getNextAudioBlock(juce::AudioSourceChannelInfo(&temp1, 0, bufferToFill.numSamples));
            s2->getNextAudioBlock(juce::AudioSourceChannelInfo(&temp2, 0, bufferToFill.numSamples));

            auto buffer1 = temp1.getReadPointer(channel, bufferToFill.startSample);
            auto buffer2 = temp2.getReadPointer(channel, bufferToFill.startSample);

            auto buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                buffer[sample] = stateholder->getState()->operation(buffer1[sample], buffer2[sample]);
        }
    }
    void Start() override
    {
        if (s1 == nullptr || s2 == nullptr)
            return;
        s2->Start();
        s1->Start();
    }
    void Stop() override
    {
        if (s1 == nullptr || s2 == nullptr)
            return;
        s1->Stop();
        s2->Stop();
    }
    StartableSource *s1;
    StartableSource *s2;
    juce::AudioBuffer<float> temp1;
    juce::AudioBuffer<float> temp2;
    StateHolder *stateholder;
};

class TriggeringSource : public StartableSource
{
public:
    class Listener
    {
    public:
        virtual void onTrigger(float amplitude) = 0;
    };
    TriggeringSource(Listener &l) : listener(l)
    {
        source = nullptr;
    };
    void setSource(StartableSource *s)
    {
        source = s;
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        if (source == nullptr)
            return;
        source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        if (source == nullptr)
            return;
        source->releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        if (source == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        // temp.setSize(juce::jmax(1, bufferToFill.buffer->getNumChannels()),
        n = n + bufferToFill.buffer->getNumSamples();

        // source->getNextAudioBlock(juce::AudioSourceChannelInfo(&temp, 0, bufferToFill.numSamples));
        // auto buffer1 = temp.getReadPointer(0, bufferToFill.startSample);
        // float number = buffer1[0];
        source->getNextAudioBlock(bufferToFill);

        if (n >= 48000 / 400)
        {
            n = 0;
            listener.onTrigger(bufferToFill.buffer->getReadPointer(0)[0]);
        }
    }
    void Start() override
    {
        if (source == nullptr)
            return;
        source->Start();
    }
    void Stop() override
    {
        if (source == nullptr)
            return;
        source->Stop();
    }

private:
    StartableSource *source;
    juce::AudioBuffer<float> temp;
    Listener &listener;
    int n = 0;
};