#pragma once
#include <JuceHeader.h>
#include "Functions.h"

template <class T>
class ValueHolder2
{
public:
    virtual T *getState() = 0;
};

class StartableSource : public juce::AudioSource
{
public:
    StartableSource(){};
    virtual void setPosition(int pos = 0) = 0;
    virtual void Stop() = 0;
    virtual int getLength() = 0;
    virtual int getCurrentPosition() = 0;
    //   virtual void setCurrentPosition(int position) = 0;
    virtual bool isPlaying()
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
    void setPosition(int p) override
    {
        source->setPosition(p);
    };
    void Stop() override
    {
        source->Stop();
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
    void setPosition(int p) override
    {
        Stop();
        transportSource.setPosition(p);
        transportSource.start();
    }
    void Stop() override
    {
        transportSource.stop();
    }

    int getCurrentPosition() override
    {
        return transportSource.getCurrentPosition();
    }
    int getLength() override
    {
        return transportSource.getTotalLength();
    }
    std::string path;
    juce::AudioTransportSource transportSource;
    juce::File file;
    //  juce::TimeSliceThread thread{"audio file preview"};
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
    void Stop() override
    {
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
    void Stop() override
    {
        if (s1 == nullptr || s2 == nullptr)
            return;
        s1->Stop();
        s2->Stop();
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
        currentTrack = 0;
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
        if (currentTrack >= numOfTracks)
        {
            currentTrack = 0;
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        auto s = sources[currentTrack];
        int length = s->getLength();
        int position = s->getCurrentPosition();
        if (position >= length)
        {
            // if the track is ended, read from the next track
            currentTrack++;
            if (currentTrack >= numOfTracks)
            {
                bufferToFill.clearActiveBufferRegion();
                return;
            }
            s = sources[currentTrack];
            s->setPosition(0);
            length = s->getLength();
            position = s->getCurrentPosition();
        }

        // write to the buffer as much data as we can
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
                output[sample] = buffer[sample];
            }
            n++;
        }

        // if the track ended in the middle of buffer
        if (currentTrackCI.numSamples != bufferToFill.numSamples)
        {
            // if it was not the last track,
            // fill the second part of block with the next track
            if (currentTrack != numOfTracks - 1)
            {
                juce::AudioSourceChannelInfo nextTrackCI(bufferToFill);
                nextTrackCI.startSample = 0;
                nextTrackCI.numSamples = bufferToFill.numSamples - currentTrackCI.numSamples;
                sources[currentTrack + 1]->setPosition();
                sources[currentTrack + 1]->getNextAudioBlock(nextTrackCI);
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
                // switch to the next track
                currentTrack++;
            }
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
            if (next_counter > n)
            {
                currentTrack = i;
                offset = n - prev_counter;
                sources[currentTrack]->setPosition(offset);
                break;
            }
            prev_counter = length_counter;
        }
    }
    void Stop() override
    {
        for (auto &s : sources)
        {
            s->Stop();
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
    int currentTrack;
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
    void Stop() override
    {
        source->Stop();
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