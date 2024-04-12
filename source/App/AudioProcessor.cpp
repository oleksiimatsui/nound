#include "AudioProcessor.h"

AudioProcessor::~AudioProcessor()
{
}

const String AudioProcessor::getName() const
{
    return "Audio processor";
}

bool AudioProcessor::acceptsMidi() const
{
    return false;
}

bool AudioProcessor::producesMidi() const
{
    return false;
}

bool AudioProcessor::isMidiEffect() const
{
    return false;
}

double AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioProcessor::setCurrentProgram(int index)
{
}

const String AudioProcessor::getProgramName(int index)
{
    return {};
}

void AudioProcessor::changeProgramName(int index, const String &newName)
{
}

void AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void AudioProcessor::releaseResources()
{
}

void AudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages)
{
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            DBG("Note number " << message.getNoteNumber());
        }
    }
}

void AudioProcessor::PlaySound()
{
}

bool AudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor *AudioProcessor::createEditor()
{
    return new AudioProcessorEditor(*this);
}

void AudioProcessor::getStateInformation(MemoryBlock &destData)
{
}

void AudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
}

AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioProcessor();
}