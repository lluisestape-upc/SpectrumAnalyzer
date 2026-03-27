#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Editor principal para el Analizador de Espectro
 */
class SpectrumAnalyzerAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor&);
    ~SpectrumAnalyzerAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    // Bucle de animación (60 FPS)
    void timerCallback() override;

private:
    // Referencia al procesador (DSP)
    SpectrumAnalyzerAudioProcessor& audioProcessor;

    // --- Componentes UI ---
    juce::TextButton freezeButton{ "FREEZE" };

    // --- Datos del Espectro ---
    // 512 puntos de resolución en pantalla para el dibujo de la FFT
    static constexpr int scopeSize = 512;
    float scopeData[scopeSize];

    // --- Inercia de los Vumeters ---
    // Guardamos el estado anterior para hacer la "caída analógica"
    float vuLeft = 0.0f;
    float vuRight = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};