#include "PluginProcessor.h"
#include "PluginEditor.h"

// Fíjate que el nombre de la clase y los argumentos coincidan perfectamente
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    juce::zeromem(scopeData, sizeof(scopeData));

    // Configurar Botón Freeze
    freezeButton.setButtonText("FREEZE");
    freezeButton.setClickingTogglesState(true);
    freezeButton.onClick = [this]() { audioProcessor.isFrozen = freezeButton.getToggleState(); };
    addAndMakeVisible(freezeButton);

    startTimerHz(60);
    setSize(900, 500);
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor() { stopTimer(); }

void SpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
    // --- 1. FÍSICA DE LOS VUMETERS (-60dB a 0dB) ---
    float dbL = juce::Decibels::gainToDecibels(audioProcessor.rmsLeft);
    float dbR = juce::Decibels::gainToDecibels(audioProcessor.rmsRight);

    // Mapeamos a una escala de dibujo de 0.0 a 1.0
    float targetL = juce::jlimit(0.0f, 1.0f, juce::jmap(dbL, -60.0f, 0.0f, 0.0f, 1.0f));
    float targetR = juce::jlimit(0.0f, 1.0f, juce::jmap(dbR, -60.0f, 0.0f, 0.0f, 1.0f));

    // Inercia analógica (Sube rápido, cae lento)
    if (targetL > vuLeft) vuLeft = targetL; else vuLeft -= 0.02f;
    if (targetR > vuRight) vuRight = targetR; else vuRight -= 0.02f;

    // --- 2. PROCESAMIENTO DEL ESPECTRO ---
    if (!audioProcessor.isFrozen && audioProcessor.getNextFFTBlockReady())
    {
        auto* fftData = audioProcessor.getFFTData();
        auto sampleRate = audioProcessor.getSampleRate();

        audioProcessor.getWindow().multiplyWithWindowingTable(fftData, audioProcessor.fftSize);
        audioProcessor.getFFT().performFrequencyOnlyForwardTransform(fftData);

        for (int i = 0; i < scopeSize; ++i)
        {
            // (AQUÍ DENTRO DEJAS TODO TU CÓDIGO ACTUAL DE LA FFT IGUAL)
            float minFreq = 20.0f;
            float maxFreq = 20000.0f;
            float freq = minFreq * std::pow(maxFreq / minFreq, i / (float)scopeSize);
            float fftIndex = freq * audioProcessor.fftSize / sampleRate;

            float rawMag = fftData[(int)fftIndex];
            float normMag = rawMag / (audioProcessor.fftSize / 2.0f);
            float dbFS = juce::Decibels::gainToDecibels(normMag);
            auto level = juce::jlimit(0.0f, 1.0f, juce::jmap(dbFS, -100.0f, 0.0f, 0.0f, 1.0f));

            if (level >= scopeData[i]) scopeData[i] = level;
            else scopeData[i] -= 0.04f;
        }
        audioProcessor.setNextFFTBlockReady(false);
    }
    repaint();
}

void SpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fondo oscuro "Studio"
    g.fillAll(juce::Colour(0xFF121417));

    auto area = getLocalBounds();
    auto w = (float)area.getWidth();
    auto h = (float)area.getHeight();

    // =======================================================
    // 1. DIBUJAR EL ESPECTRO SUAVIZADO (Bézier)
    // =======================================================
    juce::Path wavePath;
    juce::Array<juce::Point<float>> points;

    for (int i = 0; i < scopeSize; ++i)
    {
        float smoothedValue = scopeData[i];

        // Media móvil de 5 puntos para suavizar el ruido
        if (i > 1 && i < scopeSize - 2) {
            smoothedValue = (scopeData[i - 2] + scopeData[i - 1] + scopeData[i] + scopeData[i + 1] + scopeData[i + 2]) / 5.0f;
        }

        // Calculamos coordenadas en pantalla
        float x = juce::jmap((float)i, 0.0f, (float)scopeSize, 0.0f, w);
        float y = juce::jmap(smoothedValue, 0.0f, 1.0f, h, 20.0f);

        points.add({ x, y });
    }

    wavePath.startNewSubPath(0, h);
    wavePath.lineTo(points[0]);

    for (int i = 1; i < points.size(); ++i)
    {
        auto p1 = points[i - 1];
        auto p2 = points[i];

        auto midPoint = juce::Point<float>((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f);
        wavePath.quadraticTo(p1.x, p1.y, midPoint.x, midPoint.y);
    }

    wavePath.lineTo(points.getLast());
    wavePath.lineTo(w, h);
    wavePath.lineTo(0, h);
    wavePath.closeSubPath();

    g.setColour(juce::Colours::cyan.withAlpha(0.9f));
    g.strokePath(wavePath, juce::PathStrokeType(2.0f));

    juce::ColourGradient gradient(juce::Colours::cyan.withAlpha(0.2f), 0, h / 2,
        juce::Colours::transparentBlack, 0, h, false);
    g.setGradientFill(gradient);
    g.fillPath(wavePath);

    // =======================================================
    // 2. DIBUJAR VUMETER (Sweeper) - Derecha
    // =======================================================
    auto meterArea = area.removeFromRight(60).reduced(10, 40);
    g.setColour(juce::Colours::black);
    g.fillRect(meterArea);

    float valY = juce::jmap(vuLeft, 0.0f, 1.0f, (float)meterArea.getHeight(), 0.0f);
    auto fillRect = meterArea.withTop(meterArea.getY() + (int)valY);

    juce::ColourGradient grad(juce::Colours::red, meterArea.getX(), meterArea.getY(),
        juce::Colours::green, meterArea.getX(), meterArea.getBottom(), false);
    grad.addColour(0.25, juce::Colours::yellow);
    g.setGradientFill(grad);
    g.fillRect(fillRect);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    float marcasDB[] = { 0, -6, -12, -18, -30, -45, -60 };
    for (auto db : marcasDB) {
        float y = juce::jmap(db, -60.0f, 0.0f, (float)meterArea.getBottom(), (float)meterArea.getY());
        g.drawHorizontalLine((int)y, meterArea.getX() - 8, meterArea.getX());

        g.setFont(9.0f);
        g.drawText(juce::String(db), meterArea.getX() - 25, (int)y - 5, 15, 10, juce::Justification::centredRight);
    }

    // =======================================================
    // 3. DIBUJAR PANNING (L/R) - Superior
    // =======================================================
    auto panArea = area.removeFromTop(40).removeFromRight(200).reduced(10);
    g.setColour(juce::Colours::black);
    g.fillRect(panArea);

    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawVerticalLine(panArea.getX() + panArea.getWidth() / 2, panArea.getY(), panArea.getBottom());

    float panDiff = vuRight - vuLeft;
    float panPos = juce::jlimit(0.0f, (float)panArea.getWidth(),
        juce::jmap(panDiff, -1.0f, 1.0f, 0.0f, (float)panArea.getWidth()));

    g.setColour(juce::Colours::cyan);
    g.drawVerticalLine(panArea.getX() + (int)panPos, panArea.getY(), panArea.getBottom());
}

void SpectrumAnalyzerAudioProcessorEditor::resized() {
    freezeButton.setBounds(10, 10, 70, 25);
}