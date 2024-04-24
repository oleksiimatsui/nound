#pragma once
#include <JuceHeader.h>
#include "Functions.h"

template <class T>
class ValueHolder
{
public:
    virtual T *getState() = 0;
};

class StartableSource : public juce::AudioSource
{
public:
    StartableSource(){};
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual bool isPlaying() = 0;

private:
};

class SoundOutputSource : public juce::AudioSource
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
        if (source == nullptr || source->isPlaying() == false)
        {
            bufferToFill.clearActiveBufferRegion();
            //   Stop();
            return;
        }
        source->getNextAudioBlock(bufferToFill);
    };

    void Start()
    {
        if (source == nullptr)
            return;
        setAudioChannels(0, 2);
        source->Start();
    }
    void Stop()
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
    bool isPlaying() override
    {
        return source->isPlaying();
    }
    StartableSource *source;
    juce::ReverbAudioSource *r;
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
    bool isPlaying()
    {
        return transportSource.isPlaying();
    }

    std::string path;
    juce::AudioTransportSource transportSource;
    juce::File file;
    //  juce::TimeSliceThread thread{"audio file preview"};
};

class RandomSource : public StartableSource
{
public:
    RandomSource(double _t)
    {
        t = _t;
        samples_count = 0;
        n = 0;
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        samples_count = t * sampleRate;
        random.setSeed(0);
    }
    void releaseResources() override
    {
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        // Fill the required number of samples with noise between -0.125 and +0.125
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            {
                auto *buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

                buffer[sample] = random.nextFloat() * 0.25f - 0.125f;
            }
            n++;
        }
        if (n >= samples_count)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
    }
    void Start() override
    {
    }
    void Stop() override
    {
    }
    bool isPlaying() override
    {
        return n < samples_count;
    }

private:
    juce::Random random;
    double t; // time to play in seconds
    int n;
    int samples_count;
};

class Osc : public StartableSource
{
public:
    Osc(float &t, float &f, float &p, F **w) : time(t), frequency(f), phase(p), StartableSource()
    {
        samples_count = 0;
        n = 0;
        period = 0;
        N = 0;
        sampleRate = 0;
        waveform = w;
    }

    void prepareToPlay(int samplesPerBlockExpected, double _sampleRate) override
    {
        sampleRate = _sampleRate;
        period = 1.0f / sampleRate;
        samples_count = sampleRate * time;
    }
    void releaseResources() override
    {
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            auto radians = 2.0f * juce::MathConstants<double>::pi * frequency * period * n + phase;
            auto currentSample = (*waveform)->get(radians);
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            {
                auto *buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                buffer[sample] = currentSample;
            }
            n++;
            N++;
            if (frequency * period * n >= 1)
            {
                n = 0;
            }
        }
    }
    void Start() override
    {
    }
    void Stop() override
    {
    }
    bool isPlaying() override
    {
        return N <= samples_count;
    }
    float &frequency;
    float &phase;
    double period;
    int n;
    int N;
    double sampleRate;
    float time;
    int samples_count;
    F **waveform;
};

class MathAudioSource : public StartableSource
{
public:
    class State
    {
    public:
        virtual double operation(double a, double b)
        {
            return 0;
        }
    };
    class Add : public State
    {
    public:
        double operation(double a, double b) override
        {
            return a + b;
        }
    };
    class Substract : public State
    {
    public:
        double operation(double a, double b) override
        {
            return a - b;
        }
    };
    class Multiply : public State
    {
    public:
        double operation(double a, double b) override
        {
            return a * b;
        }
    };
    class Divide : public State
    {
    public:
        double operation(double a, double b) override
        {
            return a / b;
        }
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
        s1->getNextAudioBlock(juce::AudioSourceChannelInfo(&temp1, 0, bufferToFill.numSamples));
        s2->getNextAudioBlock(juce::AudioSourceChannelInfo(&temp2, 0, bufferToFill.numSamples));
        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
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

    bool isPlaying() override
    {
        return s1->isPlaying() && s2->isPlaying();
    }

    StartableSource *s1;
    StartableSource *s2;
    juce::AudioBuffer<float> temp1;
    juce::AudioBuffer<float> temp2;
    ValueHolder<State> *stateholder;
};

class ConcatenationSource : public StartableSource
{
public:
    ConcatenationSource()
    {
        is_playing = false;
        currentTrack = 0;
        numOfTracks = 0;
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        numOfTracks = sources.size();
        for (auto &s : sources)
        {
            s->prepareToPlay(samplesPerBlockExpected, sampleRate);
        }
    }
    void releaseResources() override
    {
        for (auto &s : sources)
        {
            s->releaseResources();
        }
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        auto s = sources[currentTrack];
        if (s->isPlaying())
        {
            s->getNextAudioBlock(bufferToFill);
        }
        else if (currentTrack < numOfTracks - 1)
        {
            currentTrack++;
        }
        else
        {
            is_playing = false;
            bufferToFill.clearActiveBufferRegion();
            return;
        }
    };
    void Start() override
    {
        if (sources.size() == 0)
            return;
        is_playing = true;
        sources[0]->Start();
    }
    void Stop() override
    {
        for (auto &s : sources)
        {
            s->Stop();
        }
    }
    bool isPlaying()
    {
        return is_playing;
    }

    std::vector<StartableSource *> sources;

private:
    int numOfTracks;
    int currentTrack;
    bool is_playing;
};