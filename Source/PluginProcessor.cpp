#include "PluginProcessor.h"
#include "PluginEditor.h"

SpectrumAnalyzerAudioProcessor::SpectrumAnalyzerAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{}

SpectrumAnalyzerAudioProcessor::~SpectrumAnalyzerAudioProcessor() {}

void SpectrumAnalyzerAudioProcessor::prepareToPlay(double, int)
{
    std::fill(std::begin(fifo),    std::end(fifo),    0.0f);
    std::fill(std::begin(fftData), std::end(fftData), 0.0f);
    fifoIndex         = 0;
    hopCounter        = 0;
    nextFFTBlockReady = false;
}

void SpectrumAnalyzerAudioProcessor::releaseResources() {}

void SpectrumAnalyzerAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    fifo[fifoIndex] = sample;
    fifoIndex = (fifoIndex + 1) % fftSize;

    if (++hopCounter >= hopSize)
    {
        hopCounter = 0;
        if (!nextFFTBlockReady)
        {
            // Unwrap circular buffer oldest-first into fftData
            const int n1 = fftSize - fifoIndex;
            std::memcpy(fftData,        fifo + fifoIndex, n1 * sizeof(float));
            std::memcpy(fftData + n1,   fifo,             fifoIndex * sizeof(float));
            nextFFTBlockReady = true;
        }
    }
}

void SpectrumAnalyzerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    int numSamples = buffer.getNumSamples();

    // RMS levels
    if (buffer.getNumChannels() > 0)
        rmsLeft  = buffer.getRMSLevel(0, 0, numSamples);
    if (buffer.getNumChannels() > 1)
        rmsRight = buffer.getRMSLevel(1, 0, numSamples);
    else
        rmsRight = rmsLeft;

    // Feed left channel into FFT FIFO and waveform ring
    auto* ch  = buffer.getReadPointer(0);
    int   wh  = waveformWriteHead.load(std::memory_order_relaxed);
    for (int i = 0; i < numSamples; ++i)
    {
        pushNextSampleIntoFifo(ch[i]);
        waveformRing[wh] = ch[i];
        wh = (wh + 1) % waveformSize;
    }
    waveformWriteHead.store(wh, std::memory_order_release);
}

juce::AudioProcessorEditor* SpectrumAnalyzerAudioProcessor::createEditor()
{
    return new SpectrumAnalyzerAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAnalyzerAudioProcessor();
}
