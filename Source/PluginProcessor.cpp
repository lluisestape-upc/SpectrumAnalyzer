#include "PluginProcessor.h"
#include "PluginEditor.h"

SpectrumAnalyzerAudioProcessor::SpectrumAnalyzerAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    forwardFFT(fftOrder),
    window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
}

SpectrumAnalyzerAudioProcessor::~SpectrumAnalyzerAudioProcessor() {}

void SpectrumAnalyzerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    std::fill(std::begin(fifo), std::end(fifo), 0.0f);
    fifoIndex = 0;
    nextFFTBlockReady = false;
}

void SpectrumAnalyzerAudioProcessor::releaseResources() {}

void SpectrumAnalyzerAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    if (fifoIndex == fftSize)
    {
        if (!nextFFTBlockReady)
        {
            std::memcpy(fftData, fifo, sizeof(fifo));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
}

void SpectrumAnalyzerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Calcular niveles para los vumeters
    rmsLeft = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    rmsRight = buffer.getMagnitude(1, 0, buffer.getNumSamples());

    auto* channelData = buffer.getReadPointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        pushNextSampleIntoFifo(channelData[i]);
}

juce::AudioProcessorEditor* SpectrumAnalyzerAudioProcessor::createEditor()
{
    return new SpectrumAnalyzerAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SpectrumAnalyzerAudioProcessor(); }