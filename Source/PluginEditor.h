#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// ── Theme ─────────────────────────────────────────────────────────────────────
struct ThemeColors
{
    juce::Colour bg, panel, border, accent, accentDim, warn, danger, textHi, textMid, grid;
    const char*  name;
};

static constexpr int kNumThemes = 5;
extern const ThemeColors kThemes[kNumThemes];

// ── View modes ────────────────────────────────────────────────────────────────
enum class ViewMode { Spectrum, Spectrogram, Waterfall3D, Waveform };

// ── LookAndFeel ───────────────────────────────────────────────────────────────
class ProLookAndFeel : public juce::LookAndFeel_V4
{
public:
    const ThemeColors* themePtr = nullptr;

    void applyTheme(const ThemeColors& t);
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
};

// ── Editor ────────────────────────────────────────────────────────────────────
class SpectrumAnalyzerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor&);
    ~SpectrumAnalyzerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SpectrumAnalyzerAudioProcessor& audioProcessor;

    static constexpr int scopeSize = SpectrumAnalyzerAudioProcessor::scopeSize;
    float scopeData[scopeSize] = {};

    // VU state
    float vuLeft = 0.0f, vuRight = 0.0f;
    float peakLeft = 0.0f, peakRight = 0.0f;
    int   peakHoldL = 0,  peakHoldR = 0;
    static constexpr int peakHoldFrames = 90;

    // Offscreen images
    juce::Image spectroImage;
    juce::Image waterfall3DImage;
    bool waterfall3DDirty = true;

    // State
    ViewMode currentView     = ViewMode::Spectrum;
    int      currentThemeIdx = 0;

    const ThemeColors& theme() const { return kThemes[currentThemeIdx]; }
    void applyTheme();

    // Buttons
    juce::TextButton btnSpectrum    { "SPECTRUM"  };
    juce::TextButton btnSpectrogram { "SPECTRO"   };
    juce::TextButton btnWaterfall   { "3D"        };
    juce::TextButton btnWaveform    { "WAVEFORM"  };
    juce::TextButton btnFreeze      { "FREEZE"    };
    juce::TextButton btnPreset      { "MED"       };
    juce::TextButton btnTheme       { "MIDNIGHT"  };

    int   presetIdx = 1;   // 0=slow 1=med 2=fast
    float specDecay = 0.03f;

    ProLookAndFeel laf;

    // Drawing
    void drawSpectrum      (juce::Graphics&, juce::Rectangle<int>);
    void drawSpectrogram   (juce::Graphics&, juce::Rectangle<int>);
    void drawWaterfall3D   (juce::Graphics&, juce::Rectangle<int>);
    void drawWaveform      (juce::Graphics&, juce::Rectangle<int>);
    void drawVUMeters      (juce::Graphics&, juce::Rectangle<int>);
    void drawGridX         (juce::Graphics&, juce::Rectangle<float>);
    void drawGridY         (juce::Graphics&, juce::Rectangle<float>);

    void rebuildSpectroImage     (juce::Rectangle<int>);
    void rebuildWaterfall3DImage (juce::Rectangle<int>);

    juce::Colour magnitudeToColour(float norm) const;
    juce::Font   monoFont(float h, bool bold = false) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerAudioProcessorEditor)
};
