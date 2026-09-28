#include "PluginProcessor.h"
#include "PluginEditor.h"

// ═══════════════════════════════════════════════════════════════════════════
//  Theme presets
// ═══════════════════════════════════════════════════════════════════════════
const ThemeColors kThemes[kNumThemes] =
{
    // 0 – MIDNIGHT  (dark charcoal + cyan)
    { juce::Colour(0xFF0E1014), juce::Colour(0xFF181C22), juce::Colour(0xFF252B38),
      juce::Colour(0xFF00C8FF), juce::Colour(0xFF006A86),
      juce::Colour(0xFFFFB800), juce::Colour(0xFFFF3B30),
      juce::Colour(0xFFE8EAF0), juce::Colour(0xFF6A7486), juce::Colour(0xFF1E232D),
      "MIDNIGHT" },

    // 1 – PHOSPHOR  (pure black + P31 green)
    { juce::Colour(0xFF020503), juce::Colour(0xFF06100A), juce::Colour(0xFF0D2010),
      juce::Colour(0xFF00FF50), juce::Colour(0xFF007828),
      juce::Colour(0xFFAAFF22), juce::Colour(0xFFFF4430),
      juce::Colour(0xFFC0FFC8), juce::Colour(0xFF3A6040), juce::Colour(0xFF0A1A0C),
      "PHOSPHOR" },

    // 2 – EMBER     (deep brown + orange-red)
    { juce::Colour(0xFF0D0400), juce::Colour(0xFF1C0A00), juce::Colour(0xFF301400),
      juce::Colour(0xFFFF6820), juce::Colour(0xFF882200),
      juce::Colour(0xFFFFCC40), juce::Colour(0xFFFF2020),
      juce::Colour(0xFFFFD8B0), juce::Colour(0xFF8A4820), juce::Colour(0xFF201000),
      "EMBER" },

    // 3 – VAPOR     (near-black purple + violet)
    { juce::Colour(0xFF06030E), juce::Colour(0xFF0D0820), juce::Colour(0xFF1E1440),
      juce::Colour(0xFFB060FF), juce::Colour(0xFF602090),
      juce::Colour(0xFFFF80C0), juce::Colour(0xFFFF3068),
      juce::Colour(0xFFE0D4FF), juce::Colour(0xFF6050A0), juce::Colour(0xFF10082A),
      "VAPOR" },

    // 4 – ARCTIC    (deep navy + ice blue)
    { juce::Colour(0xFF020608), juce::Colour(0xFF060E14), juce::Colour(0xFF0E2030),
      juce::Colour(0xFF80DFFF), juce::Colour(0xFF204060),
      juce::Colour(0xFF40FFCC), juce::Colour(0xFFFF4050),
      juce::Colour(0xFFD0EEFF), juce::Colour(0xFF406080), juce::Colour(0xFF081420),
      "ARCTIC" },
};

// ═══════════════════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════════════════
juce::Font SpectrumAnalyzerAudioProcessorEditor::monoFont(float h, bool bold) const
{
    return juce::Font(juce::FontOptions()
        .withName(juce::Font::getDefaultMonospacedFontName())
        .withHeight(h)
        .withStyle(bold ? "Bold" : "Regular"));
}

// ═══════════════════════════════════════════════════════════════════════════
//  ProLookAndFeel
// ═══════════════════════════════════════════════════════════════════════════
void ProLookAndFeel::applyTheme(const ThemeColors& t)
{
    themePtr = &t;
    setColour(juce::TextButton::buttonColourId,  t.panel);
    setColour(juce::TextButton::buttonOnColourId, t.accentDim);
    setColour(juce::TextButton::textColourOffId,  t.textMid);
    setColour(juce::TextButton::textColourOnId,   t.accent);
}

void ProLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                          const juce::Colour&, bool isHighlighted, bool)
{
    if (!themePtr) return;
    auto& t = *themePtr;
    auto  r = b.getLocalBounds().toFloat().reduced(0.5f);
    bool  on = b.getToggleState();

    g.setColour(on ? t.accentDim.withAlpha(0.35f)
                   : (isHighlighted ? t.border : t.panel));
    g.fillRoundedRectangle(r, 3.0f);

    g.setColour(on ? t.accent : t.border);
    g.drawRoundedRectangle(r, 3.0f, 1.0f);

    if (on)
    {
        g.setColour(t.accent);
        g.fillRect(r.getX() + 5.0f, r.getBottom() - 2.0f, r.getWidth() - 10.0f, 2.0f);
    }
}

void ProLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b,
                                    bool isHighlighted, bool)
{
    if (!themePtr) return;
    auto& t = *themePtr;
    g.setFont(juce::Font(juce::FontOptions()
        .withName(juce::Font::getDefaultMonospacedFontName())
        .withHeight(10.5f).withStyle("Bold")));
    g.setColour(b.getToggleState() ? t.accent
                                   : (isHighlighted ? t.textHi : t.textMid));
    g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Constructor / Destructor
// ═══════════════════════════════════════════════════════════════════════════
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(
    SpectrumAnalyzerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    laf.applyTheme(theme());
    setLookAndFeel(&laf);

    auto setupTab = [&](juce::TextButton& btn, ViewMode mode)
    {
        btn.setClickingTogglesState(false);
        btn.onClick = [this, &btn, mode]()
        {
            currentView = mode;
            for (auto* b : { &btnSpectrum, &btnSpectrogram, &btnWaterfall, &btnWaveform })
                b->setToggleState(false, juce::dontSendNotification);
            btn.setToggleState(true, juce::dontSendNotification);
            repaint();
        };
        addAndMakeVisible(btn);
    };

    setupTab(btnSpectrum,    ViewMode::Spectrum);
    setupTab(btnSpectrogram, ViewMode::Spectrogram);
    setupTab(btnWaterfall,   ViewMode::Waterfall3D);
    setupTab(btnWaveform,    ViewMode::Waveform);
    btnSpectrum.setToggleState(true, juce::dontSendNotification);

    btnFreeze.setClickingTogglesState(true);
    btnFreeze.onClick = [this]() { audioProcessor.isFrozen = btnFreeze.getToggleState(); };
    addAndMakeVisible(btnFreeze);

    btnPreset.setClickingTogglesState(false);
    btnPreset.onClick = [this]()
    {
        static const struct { const char* name; float decay; } presets[] =
            { { "SLOW", 0.01f }, { "MED", 0.03f }, { "FAST", 0.07f } };
        presetIdx = (presetIdx + 1) % 3;
        specDecay = presets[presetIdx].decay;
        btnPreset.setButtonText(presets[presetIdx].name);
    };
    addAndMakeVisible(btnPreset);

    btnTheme.setClickingTogglesState(false);
    btnTheme.onClick = [this]()
    {
        currentThemeIdx = (currentThemeIdx + 1) % kNumThemes;
        applyTheme();
    };
    addAndMakeVisible(btnTheme);

    spectroImage      = juce::Image(juce::Image::RGB,  1, 1, true);
    waterfall3DImage  = juce::Image(juce::Image::ARGB, 1, 1, true);

    startTimerHz(60);
    setSize(920, 540);
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void SpectrumAnalyzerAudioProcessorEditor::applyTheme()
{
    laf.applyTheme(theme());
    btnTheme.setButtonText(theme().name);
    spectroImage     = juce::Image(juce::Image::RGB,  1, 1, true);
    waterfall3DDirty = true;
    for (auto* b : { &btnSpectrum, &btnSpectrogram, &btnWaterfall,
                     &btnWaveform, &btnFreeze, &btnPreset, &btnTheme })
        b->repaint();
    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Timer
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
    // VU meters
    auto toNorm = [](float db) {
        return juce::jlimit(0.0f, 1.0f, juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f));
    };
    float dbL = juce::Decibels::gainToDecibels(audioProcessor.rmsLeft,  -100.0f);
    float dbR = juce::Decibels::gainToDecibels(audioProcessor.rmsRight, -100.0f);
    float tL = toNorm(dbL), tR = toNorm(dbR);

    if (tL > vuLeft)  vuLeft  = tL;  else vuLeft  = juce::jmax(0.0f, vuLeft  - 0.015f);
    if (tR > vuRight) vuRight = tR;  else vuRight = juce::jmax(0.0f, vuRight - 0.015f);

    if (tL >= peakLeft)  { peakLeft  = tL; peakHoldL = peakHoldFrames; }
    else if (--peakHoldL <= 0) peakLeft  = juce::jmax(0.0f, peakLeft  - 0.005f);
    if (tR >= peakRight) { peakRight = tR; peakHoldR = peakHoldFrames; }
    else if (--peakHoldR <= 0) peakRight = juce::jmax(0.0f, peakRight - 0.005f);

    // FFT
    if (!audioProcessor.isFrozen && audioProcessor.getNextFFTBlockReady())
    {
        auto* fftData = audioProcessor.getFFTData();
        auto  sr      = audioProcessor.getSampleRate();

        audioProcessor.getWindow().multiplyWithWindowingTable(fftData, audioProcessor.fftSize);
        audioProcessor.getFFT().performFrequencyOnlyForwardTransform(fftData);

        float newFrame[scopeSize];
        const float norm = 1.0f / (audioProcessor.fftSize / 2.0f);
        const int   nyq  = audioProcessor.fftSize / 2 - 2;
        for (int i = 0; i < scopeSize; ++i)
        {
            float freq  = 20.0f * std::pow(1000.0f, (float)i / (float)(scopeSize - 1));
            float idx   = juce::jlimit(0.0f, (float)nyq,
                              freq * (float)audioProcessor.fftSize / (float)sr);
            int   iIdx  = (int)idx;
            float frac  = idx - (float)iIdx;
            float raw   = (fftData[iIdx] * (1.0f - frac) + fftData[iIdx + 1] * frac) * norm;
            float db    = juce::Decibels::gainToDecibels(raw, -120.0f);
            newFrame[i] = juce::jlimit(0.0f, 1.0f, juce::jmap(db, -96.0f, 0.0f, 0.0f, 1.0f));
        }

        for (int i = 0; i < scopeSize; ++i)
        {
            if (newFrame[i] > scopeData[i]) scopeData[i] = newFrame[i];
            else scopeData[i] = juce::jmax(0.0f, scopeData[i] - specDecay);
        }

        int w = audioProcessor.spectroWriteHead;
        std::memcpy(audioProcessor.spectroBuffer[w], newFrame, scopeSize * sizeof(float));
        audioProcessor.spectroWriteHead = (w + 1) % audioProcessor.numSpectroFrames;

        waterfall3DDirty = true;
        audioProcessor.setNextFFTBlockReady(false);
    }

    repaint();
}

// ═══════════════════════════════════════════════════════════════════════════
//  Layout
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(1);
    auto header = area.removeFromTop(44);
    int  cy = header.getCentreY(), bh = 26, m = 6;

    int x = 10;
    btnSpectrum   .setBounds(x, cy-bh/2, 90, bh); x += 90+m;
    btnSpectrogram.setBounds(x, cy-bh/2, 80, bh); x += 80+m;
    btnWaterfall  .setBounds(x, cy-bh/2, 50, bh); x += 50+m;
    btnWaveform   .setBounds(x, cy-bh/2, 85, bh);

    btnPreset.setBounds(getWidth() - 256, cy-bh/2, 78, bh);
    btnTheme .setBounds(getWidth() - 172, cy-bh/2, 88, bh);
    btnFreeze.setBounds(getWidth() -  80, cy-bh/2, 70, bh);

    spectroImage     = juce::Image(juce::Image::RGB,  1, 1, true);
    waterfall3DDirty = true;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Paint
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto& T = theme();
    g.fillAll(T.bg);

    auto area = getLocalBounds().reduced(1);

    // ── Header ──────────────────────────────────────────────────────────────
    auto hdr = area.removeFromTop(44);
    g.setColour(T.panel);
    g.fillRect(hdr);
    g.setColour(T.border);
    g.drawRect(hdr, 1);

    g.setFont(monoFont(12.5f, true));
    g.setColour(T.textHi);
    g.drawText("SPECTRUM ANALYZER",
               getWidth() / 2 - 110, hdr.getCentreY() - 8, 220, 16,
               juce::Justification::centred);

    // ── VU panel (right) ────────────────────────────────────────────────────
    auto vuArea   = area.removeFromRight(64).reduced(4, 8);
    auto mainArea = area.reduced(4, 4);

    for (auto* r : { &mainArea, &vuArea })
    {
        g.setColour(T.panel);
        g.fillRoundedRectangle(r->toFloat(), 4.0f);
        g.setColour(T.border);
        g.drawRoundedRectangle(r->toFloat(), 4.0f, 1.0f);
    }

    // ── Active view ──────────────────────────────────────────────────────────
    auto view = mainArea.reduced(2);
    switch (currentView)
    {
        case ViewMode::Spectrum:    drawSpectrum   (g, view); break;
        case ViewMode::Spectrogram: drawSpectrogram(g, view); break;
        case ViewMode::Waterfall3D: drawWaterfall3D(g, view); break;
        case ViewMode::Waveform:    drawWaveform   (g, view); break;
    }

    drawVUMeters(g, vuArea);
}

// ═══════════════════════════════════════════════════════════════════════════
//  Grid helpers
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::drawGridX(juce::Graphics& g, juce::Rectangle<float> r)
{
    const float freqs[] = { 20,50,100,200,500,1000,2000,5000,10000,20000 };
    g.setFont(monoFont(9.0f));

    for (float f : freqs)
    {
        float nx = juce::mapFromLog10(f, 20.0f, 20000.0f);
        float x  = r.getX() + nx * r.getWidth();

        g.setColour(theme().grid);
        g.drawVerticalLine((int)x, r.getY(), r.getBottom());

        g.setColour(theme().textMid);
        juce::String lbl = f >= 1000.0f ? juce::String(f / 1000.0f, 0) + "k"
                                         : juce::String((int)f);
        g.drawText(lbl, (int)x - 16, (int)r.getBottom() + 3, 32, 11,
                   juce::Justification::centred);
    }
}

void SpectrumAnalyzerAudioProcessorEditor::drawGridY(juce::Graphics& g, juce::Rectangle<float> r)
{
    const float dbs[] = { 0,-12,-24,-36,-48,-60,-72,-84,-96 };
    g.setFont(monoFont(9.0f));

    for (float db : dbs)
    {
        float ny = juce::jmap(db, -96.0f, 0.0f, 1.0f, 0.0f);
        float y  = r.getY() + ny * r.getHeight();

        g.setColour(theme().grid);
        g.drawHorizontalLine((int)y, r.getX(), r.getRight());

        g.setColour(theme().textMid);
        g.drawText(juce::String((int)db), (int)r.getX() - 32, (int)y - 6, 30, 12,
                   juce::Justification::centredRight);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  Spectrum view
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::drawSpectrum(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto& T = theme();
    auto  rf = area.toFloat().reduced(38.0f, 18.0f);
    rf.setBottom(rf.getBottom() - 16.0f);

    drawGridY(g, rf);
    drawGridX(g, rf);

    juce::Array<juce::Point<float>> pts;
    pts.ensureStorageAllocated(scopeSize);

    for (int i = 0; i < scopeSize; ++i)
    {
        float sm = scopeData[i];
        if (i > 1 && i < scopeSize - 2)
            sm = (scopeData[i-2] + scopeData[i-1] + scopeData[i]
                + scopeData[i+1] + scopeData[i+2]) / 5.0f;

        float x = rf.getX() + (float)i / (scopeSize - 1) * rf.getWidth();
        float y = rf.getY() + juce::jmap(sm, 0.0f, 1.0f, rf.getHeight(), 0.0f);
        pts.add({ x, y });
    }

    juce::Path wave;
    wave.startNewSubPath(pts[0].x, rf.getBottom());
    wave.lineTo(pts[0]);
    for (int i = 1; i < pts.size(); ++i)
    {
        auto p1 = pts[i-1], p2 = pts[i];
        auto mid = juce::Point<float>((p1.x+p2.x)/2.0f, (p1.y+p2.y)/2.0f);
        wave.quadraticTo(p1.x, p1.y, mid.x, mid.y);
    }
    wave.lineTo(pts.getLast().x, rf.getBottom());
    wave.closeSubPath();

    juce::ColourGradient grad(T.accent.withAlpha(0.22f), 0, rf.getY(),
                              juce::Colours::transparentBlack, 0, rf.getBottom(), false);
    g.setGradientFill(grad);
    g.fillPath(wave);

    g.setColour(T.accent.withAlpha(0.9f));
    g.strokePath(wave, juce::PathStrokeType(1.8f));
}

// ═══════════════════════════════════════════════════════════════════════════
//  Colour maps (per theme)
// ═══════════════════════════════════════════════════════════════════════════
juce::Colour SpectrumAnalyzerAudioProcessorEditor::magnitudeToColour(float n) const
{
    n = juce::jlimit(0.0f, 1.0f, n);

    // Each theme has its own viridis-style ramp
    struct Stop { float pos; juce::Colour col; };
    const Stop maps[kNumThemes][5] =
    {
        // 0 Midnight
        {{ 0.00f, juce::Colour(0xFF08081E) }, { 0.30f, juce::Colour(0xFF004080) },
         { 0.55f, juce::Colour(0xFF00C8FF) }, { 0.80f, juce::Colour(0xFFFFFFAA) },
         { 1.00f, juce::Colour(0xFFFFFFFF) }},
        // 1 Phosphor
        {{ 0.00f, juce::Colour(0xFF000800) }, { 0.30f, juce::Colour(0xFF003810) },
         { 0.55f, juce::Colour(0xFF00C840) }, { 0.80f, juce::Colour(0xFF80FF90) },
         { 1.00f, juce::Colour(0xFFFFFFFF) }},
        // 2 Ember
        {{ 0.00f, juce::Colour(0xFF100200) }, { 0.30f, juce::Colour(0xFF601000) },
         { 0.55f, juce::Colour(0xFFFF4400) }, { 0.80f, juce::Colour(0xFFFFCC00) },
         { 1.00f, juce::Colour(0xFFFFFFFF) }},
        // 3 Vapor
        {{ 0.00f, juce::Colour(0xFF060010) }, { 0.30f, juce::Colour(0xFF3C0080) },
         { 0.55f, juce::Colour(0xFF9020E0) }, { 0.80f, juce::Colour(0xFFFF70D0) },
         { 1.00f, juce::Colour(0xFFFFFFFF) }},
        // 4 Arctic
        {{ 0.00f, juce::Colour(0xFF010408) }, { 0.30f, juce::Colour(0xFF003050) },
         { 0.55f, juce::Colour(0xFF0090B0) }, { 0.80f, juce::Colour(0xFF80E8FF) },
         { 1.00f, juce::Colour(0xFFFFFFFF) }},
    };

    const auto* m = maps[currentThemeIdx];
    for (int i = 0; i < 4; ++i)
    {
        if (n <= m[i+1].pos)
        {
            float t = (n - m[i].pos) / (m[i+1].pos - m[i].pos);
            return m[i].col.interpolatedWith(m[i+1].col, t);
        }
    }
    return m[4].col;
}

// ═══════════════════════════════════════════════════════════════════════════
//  Spectrogram view
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::rebuildSpectroImage(juce::Rectangle<int> dst)
{
    int W = dst.getWidth(), H = dst.getHeight();
    if (W < 2 || H < 2) return;

    if (spectroImage.getWidth() != W || spectroImage.getHeight() != H)
        spectroImage = juce::Image(juce::Image::RGB, W, H, false);

    int   N    = SpectrumAnalyzerAudioProcessor::numSpectroFrames;
    int   head = audioProcessor.spectroWriteHead;
    auto& buf  = audioProcessor.spectroBuffer;

    juce::Image::BitmapData bd(spectroImage, juce::Image::BitmapData::writeOnly);
    for (int px = 0; px < W; ++px)
    {
        int frameIdx = (head + (int)std::round((float)px / (W-1) * (N-1))) % N;
        const float* row = buf[frameIdx];
        for (int py = 0; py < H; ++py)
        {
            int   bin = juce::jlimit(0, scopeSize-1,
                          (int)((1.0f - (float)py / (H-1)) * (scopeSize-1)));
            bd.setPixelColour(px, py, magnitudeToColour(row[bin]));
        }
    }
}

void SpectrumAnalyzerAudioProcessorEditor::drawSpectrogram(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto  rf  = area.toFloat().reduced(38.0f, 18.0f);
    rf.setBottom(rf.getBottom() - 16.0f);

    drawGridY(g, rf);
    drawGridX(g, rf);

    auto imgBounds = rf.toNearestInt();
    rebuildSpectroImage(imgBounds);
    g.drawImage(spectroImage, imgBounds.toFloat());
    g.setColour(theme().border);
    g.drawRect(imgBounds.toFloat(), 1.0f);

    // Colour legend
    auto leg = juce::Rectangle<float>(rf.getRight() + 6.0f, rf.getY(), 10.0f, rf.getHeight());
    for (int py = 0; py < (int)leg.getHeight(); ++py)
    {
        g.setColour(magnitudeToColour(1.0f - (float)py / leg.getHeight()));
        g.drawHorizontalLine((int)(leg.getY() + py), leg.getX(), leg.getRight());
    }
    g.setFont(monoFont(8.5f));
    g.setColour(theme().textMid);
    g.drawText("0",   (int)leg.getX(), (int)leg.getY()-11,    24, 10, juce::Justification::left);
    g.drawText("-96", (int)leg.getX(), (int)leg.getBottom()+2, 24, 10, juce::Justification::left);
}

// ═══════════════════════════════════════════════════════════════════════════
//  3D Waterfall
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::rebuildWaterfall3DImage(juce::Rectangle<int> dst)
{
    int W = dst.getWidth(), H = dst.getHeight();
    if (W < 2 || H < 2) return;

    if (waterfall3DImage.getWidth() != W || waterfall3DImage.getHeight() != H)
        waterfall3DImage = juce::Image(juce::Image::ARGB, W, H, true);

    juce::Graphics ig(waterfall3DImage);
    auto& T = theme();
    ig.fillAll(T.bg);

    // Grid resolution
    constexpr int NT = 70;   // time steps rendered
    constexpr int NF = 70;   // frequency slices rendered

    const int   N    = SpectrumAnalyzerAudioProcessor::numSpectroFrames;
    const int   head = audioProcessor.spectroWriteHead;
    const int   S    = SpectrumAnalyzerAudioProcessor::scopeSize;
    const auto& buf  = audioProcessor.spectroBuffer;

    // ── Oblique 3D projection ────────────────────────────────────────────────
    // (t, f, a) all in [0,1]:
    //   t = time   (0=oldest, left;  1=newest, right)
    //   f = freq   (0=low, front;    1=high, back-right)
    //   a = amplitude (0=floor, 1=top)
    // Constraint: oy - dy - ah >= H*0.05 so the highest peak stays inside the image.
    // ox + tx + dx <= W*0.85 so the back-right corner clears the legend bar.
    const float ox = W * 0.07f, oy = H * 0.91f;  // front-left floor origin
    const float tx = W * 0.53f;                    // time axis X span
    const float dx = W * 0.24f, dy = H * 0.40f;   // depth axis (Δx rightward, Δy upward)
    const float ah = H * 0.44f;                    // amplitude height

    auto proj = [&](float t, float f, float a) -> juce::Point<float> {
        return { ox + t * tx + f * dx,
                 oy              - f * dy - a * ah };
    };

    // ── Heat colormap (blue→cyan→green→yellow→red) ───────────────────────────
    auto heat = [](float n) -> juce::Colour {
        n = juce::jlimit(0.0f, 1.0f, n);
        const float   px[] = { 0.00f, 0.15f, 0.35f, 0.55f, 0.70f, 0.85f, 1.00f };
        const uint8_t cr[] = {     0,     0,     0,     0,   230,   255,   255 };
        const uint8_t cg[] = {     0,     0,   190,   230,   230,    70,     0 };
        const uint8_t cb[] = {    60,   220,   255,    40,     0,     0,     0 };
        for (int i = 0; i < 6; ++i)
            if (n <= px[i + 1]) {
                float t = (n - px[i]) / (px[i + 1] - px[i]);
                return juce::Colour(
                    (uint8_t)(cr[i] + t * (cr[i+1] - cr[i])),
                    (uint8_t)(cg[i] + t * (cg[i+1] - cg[i])),
                    (uint8_t)(cb[i] + t * (cb[i+1] - cb[i])));
            }
        return juce::Colour(255u, 0u, 0u);
    };

    auto getAmp = [&](int ti, int fi) -> float {
        int frame = (head + ti * (N - 1) / juce::jmax(NT - 1, 1)) % N;
        int bin   = juce::jlimit(0, S - 1, fi * (S - 1) / juce::jmax(NF - 1, 1));
        return buf[frame][bin];
    };

    // ── Floor grid ────────────────────────────────────────────────────────────
    ig.setColour(T.grid.withAlpha(0.55f));
    for (int i = 0; i <= 6; ++i) {
        float t = (float)i / 6.0f;
        auto p0 = proj(t, 0.0f, 0.0f), p1 = proj(t, 1.0f, 0.0f);
        ig.drawLine(p0.x, p0.y, p1.x, p1.y, 0.6f);
    }
    for (int i = 0; i <= 8; ++i) {
        float f = (float)i / 8.0f;
        auto p0 = proj(0.0f, f, 0.0f), p1 = proj(1.0f, f, 0.0f);
        ig.drawLine(p0.x, p0.y, p1.x, p1.y, 0.6f);
    }

    // ── Left wall grid (t=0 face) ─────────────────────────────────────────────
    ig.setColour(T.grid.withAlpha(0.28f));
    for (int ai = 1; ai < 5; ++ai) {
        float a = (float)ai / 5.0f;
        auto p0 = proj(0.0f, 0.0f, a), p1 = proj(0.0f, 1.0f, a);
        ig.drawLine(p0.x, p0.y, p1.x, p1.y, 0.4f);
    }
    for (int i = 0; i <= 8; ++i) {
        float f = (float)i / 8.0f;
        auto p0 = proj(0.0f, f, 0.0f), p1 = proj(0.0f, f, 1.0f);
        ig.drawLine(p0.x, p0.y, p1.x, p1.y, 0.4f);
    }

    // ── Colored surface quads – back-to-front (high freq first) ──────────────
    for (int fi = NF - 1; fi >= 0; --fi)
    {
        for (int ti = 0; ti < NT - 1; ++ti)
        {
            float t0 = (float)ti       / (NT - 1);
            float t1 = (float)(ti + 1) / (NT - 1);
            float f0 = (float)fi       / (NF - 1);
            float f1 = (float)(fi + 1) / (NF - 1);

            float a00 = getAmp(ti,     fi);
            float a10 = getAmp(ti + 1, fi);
            float a01 = getAmp(ti,     fi + 1);
            float a11 = getAmp(ti + 1, fi + 1);
            float aAvg = (a00 + a10 + a01 + a11) * 0.25f;

            auto p00 = proj(t0, f0, a00);
            auto p10 = proj(t1, f0, a10);
            auto p11 = proj(t1, f1, a11);
            auto p01 = proj(t0, f1, a01);

            juce::Path q;
            q.startNewSubPath(p00.x, p00.y);
            q.lineTo(p10.x, p10.y);
            q.lineTo(p11.x, p11.y);
            q.lineTo(p01.x, p01.y);
            q.closeSubPath();

            ig.setColour(heat(aAvg));
            ig.fillPath(q);
            ig.setColour(juce::Colours::black.withAlpha(0.18f));
            ig.strokePath(q, juce::PathStrokeType(0.4f));
        }
    }

    // ── Box outline ───────────────────────────────────────────────────────────
    ig.setColour(T.border.brighter(0.3f));
    auto fl  = proj(0,0,0), fr  = proj(1,0,0);
    auto bl  = proj(0,1,0), br  = proj(1,1,0);
    auto flt = proj(0,0,1), frt = proj(1,0,1), blt = proj(0,1,1);
    ig.drawLine(fl.x, fl.y, fr.x, fr.y, 1.0f);
    ig.drawLine(fl.x, fl.y, bl.x, bl.y, 1.0f);
    ig.drawLine(fr.x, fr.y, br.x, br.y, 1.0f);
    ig.drawLine(bl.x, bl.y, br.x, br.y, 1.0f);
    ig.drawLine(fl.x, fl.y, flt.x, flt.y, 1.0f);
    ig.drawLine(fr.x, fr.y, frt.x, frt.y, 1.0f);
    ig.drawLine(bl.x, bl.y, blt.x, blt.y, 1.0f);
    ig.drawLine(flt.x, flt.y, frt.x, frt.y, 1.0f);
    ig.drawLine(flt.x, flt.y, blt.x, blt.y, 1.0f);

    // ── Color bar legend ──────────────────────────────────────────────────────
    const float legX = W * 0.88f, legY = H * 0.06f;
    const float legH = H * 0.76f, legW = 10.0f;
    for (int py = 0; py < (int)legH; ++py) {
        ig.setColour(heat(1.0f - (float)py / legH));
        ig.fillRect(legX, legY + (float)py, legW, 1.3f);
    }
    ig.setColour(T.border);
    ig.drawRect(juce::Rectangle<float>(legX, legY, legW, legH), 0.8f);
    ig.setFont(monoFont(8.5f));
    ig.setColour(T.textMid);
    ig.drawText("0",   (int)(legX + 13), (int)legY - 5,           24, 10, juce::Justification::left);
    ig.drawText("-96", (int)(legX + 13), (int)(legY + legH) - 5,  24, 10, juce::Justification::left);
    ig.drawText("dB",  (int)legX,        (int)(legY + legH) + 7,  30, 11, juce::Justification::left);

    // ── Frequency axis labels (left wall) ─────────────────────────────────────
    const float freqLabels[] = { 20, 100, 500, 2000, 10000, 20000 };
    ig.setFont(monoFont(8.5f));
    ig.setColour(T.textMid);
    for (float fhz : freqLabels) {
        float fn = juce::mapFromLog10(fhz, 20.0f, 20000.0f);
        auto  p  = proj(0.0f, fn, 0.0f);
        juce::String lbl = fhz >= 1000.0f ? juce::String(fhz / 1000.0f, 0) + "k"
                                           : juce::String((int)fhz);
        ig.drawText(lbl, (int)p.x - 30, (int)p.y - 5, 27, 11, juce::Justification::centredRight);
    }

    // ── Time axis labels ──────────────────────────────────────────────────────
    ig.setFont(monoFont(8.5f));
    ig.setColour(T.textMid);
    { auto p = proj(0.0f, 0.0f, 0.0f);
      ig.drawText("OLDER", (int)p.x - 18, (int)p.y + 4, 46, 11, juce::Justification::centred); }
    { auto p = proj(0.5f, 0.0f, 0.0f);
      ig.drawText("TIME >",            (int)p.x - 22, (int)p.y + 4, 44, 11, juce::Justification::centred); }
    { auto p = proj(1.0f, 0.0f, 0.0f);
      ig.drawText("NOW",   (int)p.x - 16, (int)p.y + 4, 32, 11, juce::Justification::centred); }

    waterfall3DDirty = false;
}

void SpectrumAnalyzerAudioProcessorEditor::drawWaterfall3D(juce::Graphics& g, juce::Rectangle<int> area)
{
    if (waterfall3DDirty)
        rebuildWaterfall3DImage(area);

    if (waterfall3DImage.isValid())
        g.drawImage(waterfall3DImage, area.toFloat());
}

// ═══════════════════════════════════════════════════════════════════════════
//  Waveform / Oscilloscope
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::drawWaveform(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto& T = theme();
    auto  rf = area.toFloat().reduced(38.0f, 18.0f);
    rf.setBottom(rf.getBottom() - 16.0f);

    // Amplitude grid
    const float ampMarks[] = { 1.0f, 0.5f, 0.0f, -0.5f, -1.0f };
    g.setFont(monoFont(9.0f));
    for (float a : ampMarks)
    {
        float y = rf.getY() + juce::jmap(a, -1.0f, 1.0f, rf.getHeight(), 0.0f);
        g.setColour(a == 0.0f ? T.border.brighter(0.4f) : T.grid);
        g.drawHorizontalLine((int)y, rf.getX(), rf.getRight());
        g.setColour(T.textMid);
        g.drawText(juce::String(a, 1), (int)rf.getX()-34, (int)y-6, 32, 12,
                   juce::Justification::centredRight);
    }

    // Time grid
    const int numDivs = 8;
    for (int d = 1; d < numDivs; ++d)
    {
        float x = rf.getX() + (float)d / numDivs * rf.getWidth();
        g.setColour(T.grid);
        g.drawVerticalLine((int)x, rf.getY(), rf.getBottom());
    }

    // Waveform from ring buffer
    int  waveN = SpectrumAnalyzerAudioProcessor::waveformSize;
    int  wHead = audioProcessor.waveformWriteHead.load(std::memory_order_acquire);

    juce::Path wave;
    bool first = true;
    for (int i = 0; i < waveN; ++i)
    {
        float amp = audioProcessor.waveformRing[(wHead + i) % waveN];
        float x   = rf.getX() + (float)i / (waveN - 1) * rf.getWidth();
        float y   = rf.getY() + juce::jmap(amp, -1.0f, 1.0f, rf.getHeight(), 0.0f);
        if (first) { wave.startNewSubPath(x, y); first = false; }
        else wave.lineTo(x, y);
    }

    // Glow + stroke
    g.setColour(T.accent.withAlpha(0.20f));
    g.strokePath(wave, juce::PathStrokeType(3.5f));
    g.setColour(T.accent.withAlpha(0.90f));
    g.strokePath(wave, juce::PathStrokeType(1.5f));

    // Labels
    g.setFont(monoFont(9.0f));
    g.setColour(T.textMid);
    g.drawText("TIME →",
               (int)rf.getRight() - 52, (int)rf.getBottom() + 3, 50, 11,
               juce::Justification::centredRight);
    g.drawText("AMP",
               (int)rf.getX() - 34, (int)rf.getY(), 32, 11,
               juce::Justification::centredRight);
}

// ═══════════════════════════════════════════════════════════════════════════
//  VU Meters
// ═══════════════════════════════════════════════════════════════════════════
void SpectrumAnalyzerAudioProcessorEditor::drawVUMeters(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto& T   = theme();
    int   barW = (area.getWidth() - 10) / 2;
    int   barH = area.getHeight() - 22;
    int   barY = area.getY() + 4;
    int   numSegs = 32;

    auto drawBar = [&](int x, float level, float peak, const char* label)
    {
        juce::Rectangle<int> bg(x, barY, barW, barH);
        g.setColour(juce::Colour(0xFF080B0E));
        g.fillRect(bg);

        int filled = (int)(level * numSegs);
        int segH   = (barH - numSegs) / numSegs;

        for (int s = 0; s < numSegs; ++s)
        {
            int sy = barY + barH - (s + 1) * (segH + 1);
            float norm = (float)s / numSegs;
            juce::Colour c = (s < filled)
                ? (norm > 0.84f ? T.danger : norm > 0.68f ? T.warn : T.accent)
                : T.grid;
            g.setColour(c);
            g.fillRect(x, sy, barW, segH);
        }

        // Peak tick
        if (peak > 0.01f)
        {
            int py = barY + barH - (int)(peak * barH) - 2;
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.fillRect(x, py, barW, 2);
        }

        // dB ticks
        g.setFont(monoFont(8.0f));
        const float ticks[] = { 0,-6,-12,-18,-30,-48 };
        for (float db : ticks)
        {
            float yn = juce::jmap(db, -60.0f, 0.0f, (float)barH, 0.0f);
            int   iy = barY + barH - (int)yn;
            if (db == 0 || db == -12 || db == -30)
            {
                g.setColour(T.textMid);
                g.drawText(db == 0 ? "0" : juce::String((int)db),
                           x, iy - 6, barW, 12, juce::Justification::centred);
            }
        }

        g.setColour(T.textMid);
        g.setFont(monoFont(9.0f, true));
        g.drawText(label, x, area.getBottom() - 16, barW, 14, juce::Justification::centred);
    };

    drawBar(area.getX(),           vuLeft,  peakLeft,  "L");
    drawBar(area.getX() + barW + 8, vuRight, peakRight, "R");
}
