#pragma once
#include <JuceHeader.h>

class StartableSource : public juce::AudioSource
{
public:
    virtual void Start() = 0;
};

class SoundOutputSource : public StartableSource
{
public:
    SoundOutputSource(StartableSource &s) : source(s){

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
        source.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    void releaseResources() override
    {
        source.releaseResources();
    }
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        source.getNextAudioBlock(bufferToFill);
    };

    void Start() override
    {
        setAudioChannels(0, 1);
        source.Start();
    }

private:
    StartableSource &source;
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
        // file = juce::File("D://recordings//wearing_the_inside_out.mp3");
        file = juce::File(filepath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        juce::AudioFormatReader *reader = formatManager.createReaderFor(file);
        jassert((reader != nullptr));
        juce::AudioFormatReaderSource *readerSource = new juce::AudioFormatReaderSource(reader, true);
        thread.startThread(juce::Thread::Priority::normal);
        transportSource.setSource(readerSource,
                                  32768,   // tells it to buffer this many samples ahead
                                  &thread, // this is the background thread to use for reading-ahead
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
    };
    void Start() override
    {
        transportSource.setPosition(0);
        transportSource.start();
    }

    juce::AudioTransportSource transportSource;
    juce::File file;
    juce::TimeSliceThread thread{"audio file preview"};
};

class AudioApp : public juce::AudioSource
{
public:
    AudioApp() : deviceManager(defaultDeviceManager)
    {
        file = juce::File("D://recordings//wearing_the_inside_out.mp3");
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        juce::AudioFormatReader *reader = formatManager.createReaderFor(file);
        jassert((reader != nullptr));
        juce::AudioFormatReaderSource *readerSource = new juce::AudioFormatReaderSource(reader, true);

        thread.startThread(juce::Thread::Priority::normal);
        transportSource.setSource(readerSource,
                                  32768,   // tells it to buffer this many samples ahead
                                  &thread, // this is the background thread to use for reading-ahead
                                  readerSource->getAudioFormatReader()->sampleRate);
    }
    ~AudioApp()
    {
    }
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        random.getSeed();
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
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            {
                float *buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                    buffer[sample] = buffer[sample] + 0.5 * (random.nextFloat() * 0.25f - 0.125f);
            }
        }
    };
    void setAudioChannels(int numInputChannels, int numOutputChannels, const juce::XmlElement *const xml = nullptr)
    {
        juce::String audioError;
        audioError = deviceManager.initialise(numInputChannels, numOutputChannels, xml, true);
        jassert(audioError.isEmpty());
        deviceManager.addAudioCallback(&audioSourcePlayer);
        audioSourcePlayer.setSource(this);
    }

    void start()
    {
        setAudioChannels(0, 1);
        transportSource.setPosition(0);
        transportSource.start();
    }

private:
    juce::AudioDeviceManager &deviceManager;
    juce::AudioDeviceManager defaultDeviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    juce::Random random;
    juce::File file;
    juce::TimeSliceThread thread{"audio file preview"};
};