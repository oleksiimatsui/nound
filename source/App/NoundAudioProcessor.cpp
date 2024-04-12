#include "NoundAudioProcessor.h"

NoundProcessor::NoundProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    audioFormatManager.registerBasicFormats();
    juce::File audioFile("D://recordings//wearing_the_inside_out.mp3");
    auto *reader = audioFormatManager.createReaderFor(audioFile);
    if (reader != nullptr)
    {
        // Create an AudioFormatReaderSource to read audio data from the reader
        juce::AudioFormatReaderSource *readerSource = new juce::AudioFormatReaderSource(reader, true);

        // Set the AudioFormatReaderSource as the source for the transportSource
        transportSource.setSource(readerSource, 0, nullptr, reader->sampleRate);

        // Start playback
        transportSource.start();
    }
}

NoundProcessor::~NoundProcessor()
{
}

const juce::String NoundProcessor::getName() const
{
    return "Nound Audio Processor";
}

bool NoundProcessor::acceptsMidi() const
{
    return false;
}

bool NoundProcessor::producesMidi() const
{
    return false;
}

bool NoundProcessor::isMidiEffect() const
{
    return false;
}

double NoundProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoundProcessor::getNumPrograms()
{
    return 1;
}

int NoundProcessor::getCurrentProgram()
{
    return 0;
}

void NoundProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String NoundProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void NoundProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

void NoundProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
}

void NoundProcessor::releaseResources()
{
}

bool NoundProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    return true;
}

void NoundProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                  juce::MidiBuffer &midiMessages)
{
    // If the audio file is playing, mix its output with the input buffer
    if (transportSource.isPlaying())
    {

        if (buffer.getNumSamples() == 0)
            return;

        juce::AudioBuffer<float> audioFileBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        audioFileBuffer.clear();

        juce::AudioSourceChannelInfo channelInfo(buffer);
        transportSource.getNextAudioBlock(channelInfo);

        // transportSource.getNextAudioBlock(juce::AudioSourceChannelInfo(audioFileBuffer));

        // for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        // {
        //     buffer.addFrom(channel, 0, audioFileBuffer, channel, 0, buffer.getNumSamples());
        // }
    }
    else
    {
        // If the audio file is not playing, pass the input buffer through without processing
        // (You can add any processing logic here if needed)
    }

    // if (readerSource.get() == nullptr)
    // {
    //     bufferToFill.clearActiveBufferRegion();
    //     return;
    // }

    // transportSource.getNextAudioBlock(buffer);
}

bool NoundProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *NoundProcessor::createEditor()
{
    return nullptr;
}

void NoundProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::MemoryOutputStream out{destData, false};
}

void NoundProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    juce::MemoryInputStream in{data, static_cast<size_t>(sizeInBytes), false};
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new NoundProcessor();
}