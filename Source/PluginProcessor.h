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

    // --- Interfaz para la UI ---
    static constexpr int fftOrder = 11; // 2048 muestras para respuesta más rápida
    static constexpr int fftSize = 1 << fftOrder;

    void pushNextSampleIntoFifo(float sample) noexcept;
    float* getFFTData() { return fftData; }
    bool getNextFFTBlockReady() const { return nextFFTBlockReady; }
    void setNextFFTBlockReady(bool ready) { nextFFTBlockReady = ready; }

    juce::dsp::FFT& getFFT() { return forwardFFT; }
    juce::dsp::WindowingFunction<float>& getWindow() { return window; }

    std::atomic<bool> isFrozen{ false }; // Para el botón Freeze
    float rmsLeft = 0.0f;
    float rmsRight = 0.0f;

    // Métodos obligatorios
    const juce::String getName() const override { return "Spectrum Analyzer"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}
    void getStateInformation(juce::MemoryBlock& destData) override {}
    void setStateInformation(const void* data, int sizeInBytes) override {}

private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessor)
};