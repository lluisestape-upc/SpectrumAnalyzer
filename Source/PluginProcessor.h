#pragma once
#include <JuceHeader.h>

class SpectrumAnalyzerAudioProcessor : public juce::AudioProcessor
{
public:
    SpectrumAnalyzerAudioProcessor();
    ~SpectrumAnalyzerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // FFT config
    static constexpr int fftOrder  = 13;
    static constexpr int fftSize   = 1 << fftOrder; // 8192
    static constexpr int hopSize   = fftSize / 4;    // 2048 – 75% overlap
    static constexpr int scopeSize = 1024;

    // Spectrogram history: numFrames rows, each scopeSize bins wide
    static constexpr int numSpectroFrames = 200;

    // Waveform ring buffer for oscilloscope (UI reads this)
    static constexpr int waveformSize = 2048;
    float waveformRing[waveformSize] = {};
    std::atomic<int> waveformWriteHead{ 0 };

    void pushNextSampleIntoFifo(float sample) noexcept;

    float* getFFTData()                  { return fftData; }
    bool   getNextFFTBlockReady() const  { return nextFFTBlockReady; }
    void   setNextFFTBlockReady(bool v)  { nextFFTBlockReady = v; }

    juce::dsp::FFT&                         getFFT()    { return forwardFFT; }
    juce::dsp::WindowingFunction<float>&    getWindow() { return window; }

    // Spectrogram ring buffer (written from UI thread after FFT)
    // Row index cycles 0..numSpectroFrames-1, newest = spectroWriteHead-1
    float spectroBuffer[numSpectroFrames][scopeSize] = {};
    int   spectroWriteHead = 0;

    std::atomic<bool> isFrozen{ false };
    float rmsLeft  = 0.0f;
    float rmsRight = 0.0f;

    const juce::String getName() const override            { return "Spectrum Analyzer"; }
    bool acceptsMidi()  const override                     { return false; }
    bool producesMidi() const override                     { return false; }
    bool isMidiEffect() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0.0; }
    int  getNumPrograms()  override                        { return 1; }
    int  getCurrentProgram() override                      { return 0; }
    void setCurrentProgram(int) override                   {}
    const juce::String getProgramName(int) override        { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override  {}
    void setStateInformation(const void*, int) override    {}

private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    float fifo[fftSize]        = {};
    float fftData[2 * fftSize] = {};
    int   fifoIndex            = 0;
    int   hopCounter           = 0;
    bool  nextFFTBlockReady    = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessor)
};
