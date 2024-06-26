#pragma once
#include <JuceHeader.h>
#include "Functions.h"

class PositionableSource : public juce::AudioSource
{
public:
    PositionableSource(){};

    virtual void setPosition(int pos = 0) = 0;
    virtual int getLength() = 0;
    virtual float getLengthInSeconds() = 0;
    virtual int getCurrentPosition() = 0;
    void setPositionInSeconds(float position, double sampleRate)
    {
        //  start_position_in_seconds = position;
        int start_position = position * sampleRate;
        setPosition(start_position);
    }
    bool isPlaying()
    {
        return getCurrentPosition() < getLength();
    }
};

class Player : private juce::AudioSource
{
public:
    Player(int _sample_rate, int _samples_per_block)
    {
        source = nullptr;
        offset_cof = 0;
        sample_rate = _sample_rate;
        samples_per_block = _samples_per_block;
        playing = false;
    };

    void setSource(PositionableSource *s)
    {
        source = s;
    };

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
    }

    void setAudioChannels(int numInputChannels, int numOutputChannels, const juce::XmlElement *const xml = nullptr)
    {
        juce::String audioError;
        audioError = deviceManager.initialise(numInputChannels, numOutputChannels, xml, true);
        jassert(audioError.isEmpty());
        deviceManager.addAudioCallback(&audioSourcePlayer);
        audioSourcePlayer.setSource(this);
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
        playing = true;
        source->prepareToPlay(samples_per_block, sample_rate);
        source->setPosition(offset_cof * sample_rate * source->getLengthInSeconds());
        setAudioChannels(0, 2);
    }
    void resume()
    {
        if (source == nullptr)
            return;
        playing = true;
        setAudioChannels(0, 2);
    }
    void setPosition(float coefficient)
    {
        offset_cof = coefficient;
        // if (source == nullptr)
        //     return;
        // source->setPosition(p);
        // if (playing == true)
        // {
        //     Start();
        // }
    }
    void Stop()
    {
        // if (source == nullptr)
        //     return;
        //   source->Stop();

        playing = false;

        //  if(audioSourcePlayer.isPla)
        // if (deviceManager.getAudioCallbackLock().tryEnter())
        //  {
        audioSourcePlayer.setSource(nullptr);
        deviceManager.closeAudioDevice();
        deviceManager.removeAudioCallback(&audioSourcePlayer);
        // }
    }

    int getLength()
    {
        if (source == nullptr)
            return 0;
        return source->getLength();
    }
    float getLengthInSeconds()
    {
        if (source == nullptr)
            return 0;
        return source->getLengthInSeconds();
    }
    void setPositionInSeconds(float offset, double sampleRate)
    {
        if (source == nullptr)
            return;
        source->setPositionInSeconds(offset, sampleRate);
    }

private:
    PositionableSource *source;
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    float offset_cof;
    int sample_rate;
    int samples_per_block;
    bool playing;
};

class ReverbSource : public PositionableSource
{
public:
    ReverbSource()
    {
        source = nullptr;
        r = nullptr;
    }
    void setSource(PositionableSource *s)
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
    float getLengthInSeconds() override
    {
        return source->getLengthInSeconds();
    }
    PositionableSource *source;
    juce::ReverbAudioSource *r;
};

class FileSource : public PositionableSource
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
        transportSource.start();

        // thread.startThread();
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
    float getLengthInSeconds() override
    {
        return transportSource.getLengthInSeconds();
    }
    ~FileSource() override
    {
        transportSource.stop();
        thread.stopThread(-1);
        transportSource.setSource(nullptr, 0, nullptr, 0);
    }
    std::string path;
    juce::AudioTransportSource transportSource;
    juce::File file;
    double sample_rate;
    juce::TimeSliceThread thread{"audio file reading thread"};
};

class Osc : public PositionableSource
{
public:
    Osc(float &t, float &f, float &p, F **w) : time(t), frequency(f), phase(p), PositionableSource()
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
    float getLengthInSeconds() override
    {
        return time;
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

class MathAudioSource : public PositionableSource
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
        if (s2 != nullptr)
            s2->prepareToPlay(samplesPerBlockExpected, sampleRate);
        if (s1 != nullptr)
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
        temp1.setSize(juce::jmax(1, bufferToFill.buffer->getNumChannels()),
                      bufferToFill.buffer->getNumSamples());
        temp2.setSize(juce::jmax(1, bufferToFill.buffer->getNumChannels()),
                      bufferToFill.buffer->getNumSamples());
        if (s1 != nullptr)
            s1->getNextAudioBlock(juce::AudioSourceChannelInfo(&temp1, 0, bufferToFill.numSamples));
        if (s2 != nullptr)
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
        if (s1 == nullptr)
            return 0;
        return s1->getCurrentPosition();
    }
    int getLength() override
    {
        return std::min(s1->getLength(), s2->getLength());
    }
    float getLengthInSeconds() override
    {
        return std::min(s1->getLengthInSeconds(), s2->getLengthInSeconds());
    }
    PositionableSource *s1;
    PositionableSource *s2;
    juce::AudioBuffer<float> temp1;
    juce::AudioBuffer<float> temp2;
    std::unique_ptr<State> *state;
};

class ConcatenationSource : public PositionableSource
{
public:
    ConcatenationSource()
    {
        length = 0;
        numOfTracks = 0;
        track_number = 0;
        global_position = 0;
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
        int remaining_samples = bufferToFill.buffer->getNumSamples();
        while (remaining_samples > 0)
        {
            auto source = sources[track_number];
            juce::AudioSourceChannelInfo currentTrackChannelInfo(bufferToFill);
            int track_remaining_samples = source->getLength() - source->getCurrentPosition();
            int num_samples = juce::jmin(remaining_samples, track_remaining_samples);
            currentTrackChannelInfo.startSample = 0;
            currentTrackChannelInfo.numSamples = num_samples;
            source->getNextAudioBlock(currentTrackChannelInfo);
            for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            {
                auto *buffer = currentTrackChannelInfo.buffer->getReadPointer(channel, 0);
                auto *output = bufferToFill.buffer->getWritePointer(channel, 0);
                std::copy(buffer, buffer + num_samples, output);
            }
            global_position += num_samples;
            remaining_samples -= num_samples;
            if (track_remaining_samples == 0)
            {
                if (sources.size() > (track_number + 1))
                {
                    track_number++;
                }
                else if (remaining_samples > 0 || num_samples == 0)
                {
                    // Fill remaining samples with zeros
                    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                    {
                        auto *output = bufferToFill.buffer->getWritePointer(channel, 0);
                        std::fill(output, output + remaining_samples, 0.0f);
                    }
                    global_position = getLength() + 1;
                    return;
                }
            }
        }
    }

    void setPosition(int p) override
    {
        if (sources.size() == 0)
            return;
        int tracks_length_sum = 0;

        for (int i = 0; i < sources.size(); i++)
        {
            int length = sources[i]->getLength();
            // source is current
            if (tracks_length_sum <= p && tracks_length_sum + length >= p)
            {
                track_number = i;
                sources[i]->setPosition(p - tracks_length_sum);
            }
            else
            {
                sources[i]->setPosition(0);
            }
            tracks_length_sum += length;
        }

        global_position = p;
    }

    int getCurrentPosition() override
    {
        return global_position;
    }
    int getLength() override
    {
        return length;
    }
    float getLengthInSeconds() override
    {
        float sec = 0;

        for (auto &s : sources)
        {
            sec += s->getLengthInSeconds();
        }
        return sec;
    }
    void setSources(const std::vector<PositionableSource *> &_sources)
    {
        sources = _sources;
    }

private:
    int numOfTracks;
    int track_number;
    int global_position;
    int length;
    std::vector<PositionableSource *> sources;
    //  int offset;
};

class RepeatSource : public PositionableSource
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
    float getLengthInSeconds() override
    {
        return seconds;
    }

    PositionableSource *source;

private:
    int n;
    int length;
    float &seconds;
};

class TrimSource : public PositionableSource
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
    float getLengthInSeconds() override
    {
        return seconds;
    }

    PositionableSource *source;

private:
    int n;
    int length;
    int start;
    float &seconds;
    float &start_seconds;
};

class ResamplingAudioSource : public PositionableSource
{
public:
    ResamplingAudioSource(PositionableSource *source, float &samplesCoefficient) : samplesInPerOutputSample(samplesCoefficient)
    {
        input = source;
        resampling.reset(new juce::ResamplingAudioSource(input, false, 2));
        resampling->setResamplingRatio(samplesInPerOutputSample);
    }
    void updateCoefficient()
    {
        resampling->setResamplingRatio(samplesInPerOutputSample);
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        resampling->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        resampling->releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        resampling->getNextAudioBlock(bufferToFill);
    };
    void setPosition(int p) override
    {
        input->setPosition(p);
    };
    int getCurrentPosition() override
    {
        return (float)input->getCurrentPosition() / samplesInPerOutputSample;
    }
    int getLength() override
    {
        return (float)input->getLength() / samplesInPerOutputSample;
    }
    float getLengthInSeconds() override
    {
        return input->getLengthInSeconds() / samplesInPerOutputSample;
    }

private:
    std::unique_ptr<juce::ResamplingAudioSource> resampling;
    PositionableSource *input;
    float &samplesInPerOutputSample;
};

class FilterSource : public PositionableSource
{
public:
    FilterSource(float &f0_, float &f1_) : f0(f0_), f1(f1_)
    {
        source = nullptr;
        r = nullptr;
    }
    ~FilterSource() override
    {
        delete r;
        r = nullptr;
    }
    void setSource(PositionableSource *s)
    {
        source = s;
        r = new juce::IIRFilterAudioSource(s, false);
    }
    void prepareToPlay(int samplesPerBlockExpected, double _sampleRate) override
    {
        sampleRate = _sampleRate;
        update();
        r->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void update()
    {
        int frequency = std::abs(f0 + f1) / 2;
        double Q = frequency / (std::abs(f0 - f1) == 0 ? 6 : std::abs(f0 - f1));
        c = c.makeBandPass(sampleRate, frequency, Q);
        r->setCoefficients(c);
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
    float getLengthInSeconds() override
    {
        return source->getLengthInSeconds();
    }

private:
    PositionableSource *source;
    juce::IIRFilterAudioSource *r;
    juce::IIRCoefficients c;
    float &f0;
    float &f1;
    int sampleRate;
};