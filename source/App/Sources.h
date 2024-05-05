#pragma once
#include <JuceHeader.h>
#include "Functions.h"

class StartableSource : public juce::AudioSource
{
public:
    StartableSource(){};
    virtual void setPosition(int pos = 0) = 0;
    virtual int getLength() = 0;
    virtual int getCurrentPosition() = 0;
    bool isPlaying()
    {
        return getCurrentPosition() < getLength();
    }

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
        source->setPosition(0);
    }
    void Stop()
    {
        if (source == nullptr)
            return;
        //   source->Stop();
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
    void setPosition(int p) override
    {
        source->setPosition(p);
    };
    int getCurrentPosition() override
    {
        return source->getCurrentPosition();
    }
    int getLength() override
    {
        return source->getLength();
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
    bool setFile(std::string filepath)
    {
        transportSource.stop();
        thread.stopThread(-1);

        path = filepath;
        file = juce::File(filepath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        juce::AudioFormatReader *reader = formatManager.createReaderFor(file);
        if (reader == nullptr)
            return false;
        thread.startThread();
        juce::AudioFormatReaderSource *readerSource = new juce::AudioFormatReaderSource(reader, true);
        //   thread.startThread(juce::Thread::Priority::normal);
        if (readerSource->getAudioFormatReader() != false)
        {
            transportSource.setSource(readerSource,
                                      32768, &thread,
                                      readerSource->getAudioFormatReader()->sampleRate);
            setPosition(0);
            return true;
        }
        else
        {
            return false;
        }
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        sample_rate = sampleRate;
        thread.startThread();
    }
    void releaseResources() override
    {
        transportSource.releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        //  if (transportSource.isPlaying())
        //   {
        transportSource.getNextAudioBlock(bufferToFill);
        //  }
        //  else
        //   {
        //      bufferToFill.clearActiveBufferRegion();
        //       return;
        //  }
    };
    void setPosition(int p) override
    {
        transportSource.setPosition(p / (double)sample_rate);
        transportSource.start();
    }
    int getCurrentPosition() override
    {
        return (double)transportSource.getCurrentPosition() * (double)sample_rate;
    }
    int getLength() override
    {
        return transportSource.getTotalLength();
    }
    std::string path;
    juce::AudioTransportSource transportSource;
    juce::File file;
    double sample_rate;
    juce::TimeSliceThread thread{"audio file reading thread"};
};

class Osc : public StartableSource
{
public:
    Osc(float &t, float &f, float &p, F **w) : time(t), frequency(f), phase(p), StartableSource()
    {
        samples_count = 0;
        n = 0;
        period = 0;
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
            if (isPlaying())
                n++;
        }
    }
    void setPosition(int p) override
    {
        n = p;
    }
    int getCurrentPosition() override
    {
        return n;
    }
    int getLength() override
    {
        return samples_count;
    }

private:
    float &frequency;
    float &phase;
    double period;
    int n;
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
                buffer[sample] = state->get()->operation(buffer1[sample], buffer2[sample]);
        }
    }
    void setPosition(int p) override
    {
        if (s1 == nullptr || s2 == nullptr)
            return;
        s2->setPosition(p);
        s1->setPosition(p);
    }

    int getCurrentPosition() override
    {
        return s1->getCurrentPosition();
    }
    int getLength() override
    {
        return std::min(s1->getLength(), s2->getLength());
    }

    StartableSource *s1;
    StartableSource *s2;
    juce::AudioBuffer<float> temp1;
    juce::AudioBuffer<float> temp2;
    std::unique_ptr<State> *state;
};

class ConcatenationSource : public StartableSource
{
public:
    ConcatenationSource()
    {
        length = 0;
        offset = 0;
        numOfTracks = 0;
        n = 0;
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        numOfTracks = sources.size();
        length = 0;

        for (auto &s : sources)
        {
            s->prepareToPlay(samplesPerBlockExpected, sampleRate);
            length += s->getLength();
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
        int currentTrack = 0;
        // find current track
        for (int i = 0; i < numOfTracks; i++)
        {
            auto s = sources[i];
            if (s->getCurrentPosition() < s->getLength())
            {
                currentTrack = i;
                break;
            }
        }

        // fill buffer with tracks
        int last_buffer_size = 0;
        for (int i = currentTrack; i < numOfTracks; i++)
        {
            auto s = sources[i];
            if (i != currentTrack)
            {
                // start every new track from beginning
                sources[i]->setPosition(0);
            }
            juce::AudioSourceChannelInfo currentTrackCI(bufferToFill);
            currentTrackCI.startSample = 0;
            currentTrackCI.numSamples = juce::jmin(bufferToFill.numSamples, s->getLength() - s->getCurrentPosition());
            s->getNextAudioBlock(currentTrackCI);
            for (auto sample = 0; sample < currentTrackCI.numSamples; ++sample)
            {
                for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                {
                    auto buffer = currentTrackCI.buffer->getReadPointer(channel, bufferToFill.startSample);
                    auto output = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                    output[last_buffer_size + sample] = buffer[sample];
                }
                n++;
            }
            last_buffer_size = currentTrackCI.numSamples;
            // if current track fills the buffer, break
            if (currentTrackCI.numSamples == bufferToFill.numSamples)
                break;
        }
    }

    void setPosition(int p) override
    {
        if (sources.size() == 0)
            return;
        //     n = 0;
        //     currentTrack = 0;
        // return;
        n = p;
        int length_counter = 0;
        int prev_counter = 0;
        for (int i = 0; i + 1 < numOfTracks; i++)
        {
            length_counter += sources[i]->getLength();
            int next_counter = length_counter + sources[i + 1]->getLength();
            if (next_counter <= n)
            {
            }
            else
            {
                int currentTrack = i;
                offset = n - prev_counter;
                sources[currentTrack]->setPosition(offset);
                break;
            }
        }
    }

    int getCurrentPosition() override
    {
        return n;
    }
    int getLength() override
    {
        return length;
    }

    std::vector<StartableSource *> sources;

private:
    int numOfTracks;
    int n;
    int length;
    int offset;
};

class RepeatSource : public StartableSource
{
public:
    RepeatSource(float &t) : seconds(t)
    {
        source = nullptr;
        n = 0;
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        length = 0;
        length = sampleRate * seconds;
        source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        source->releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        // write to output till the audio not finished
        juce::AudioSourceChannelInfo currentTrackCI(bufferToFill);
        currentTrackCI.startSample = 0;
        currentTrackCI.numSamples = juce::jmin(bufferToFill.numSamples, source->getLength() - source->getCurrentPosition());
        source->getNextAudioBlock(currentTrackCI);
        for (auto sample = 0; sample < currentTrackCI.numSamples; ++sample)
        {
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            {
                auto buffer = currentTrackCI.buffer->getReadPointer(channel, bufferToFill.startSample);
                auto output = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                output[sample] = buffer[sample];
            }
            n++;
        }

        if (source->getCurrentPosition() >= source->getLength())
        {
            // if the track is ended, start it from beginning
            source->setPosition(0);
        }

        // if the track ended in the middle of buffer
        if (currentTrackCI.numSamples != bufferToFill.numSamples)
        {
            // if it was not the end
            // fill the second part of block with the beginning of the track
            if (n < length)
            {
                juce::AudioSourceChannelInfo nextTrackCI(bufferToFill);
                nextTrackCI.startSample = 0;
                nextTrackCI.numSamples = bufferToFill.numSamples - currentTrackCI.numSamples;
                source->setPosition(0);
                source->getNextAudioBlock(nextTrackCI);
                for (auto sample = currentTrackCI.numSamples; sample < bufferToFill.numSamples; ++sample)
                {
                    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                    {
                        auto buffer = nextTrackCI.buffer->getReadPointer(channel, 0);
                        auto output = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                        output[sample] = buffer[sample];
                    }
                    n++;
                }
                // switch to the beginning of the track
                source->setPosition(0);
            }
            else
            {
                // if it is the end, fill with zeros
                for (auto sample = currentTrackCI.numSamples; sample < bufferToFill.numSamples; ++sample)
                {
                    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                    {
                        auto output = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                        output[sample] = 0;
                    }
                    n++;
                }
            }
        }
    }

    void setPosition(int p) override
    {
        if (source == nullptr)
            return;
        source->setPosition(p % source->getLength());
        n = p;
    }

    int getCurrentPosition() override
    {
        return n;
    }
    int getLength() override
    {
        return length;
    }

    StartableSource *source;

private:
    int n;
    int length;
    float &seconds;
};

class TrimSource : public StartableSource
{
public:
    TrimSource(float &start_at, float &t) : seconds(t), start_seconds(start_at)
    {
        source = nullptr;
        n = 0;
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        length = sampleRate * seconds;
        start = sampleRate * start_seconds;
        source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        source->releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        source->getNextAudioBlock(bufferToFill);
        n += bufferToFill.numSamples;
    }

    void setPosition(int p) override
    {
        if (source == nullptr)
            return;
        source->setPosition(p + start);
        n = p;
    }
    int getCurrentPosition() override
    {
        return n;
    }
    int getLength() override
    {
        return length;
    }

    StartableSource *source;

private:
    int n;
    int length;
    int start;
    float &seconds;
    float &start_seconds;
};