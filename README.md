# SPECTRUM

A real-time analyser plugin (VST3) built with C++ and JUCE. One 8192-point FFT feeds four views of the same signal: live spectrum, scrolling spectrogram, 3D waterfall and oscilloscope.

<img width="1362" height="794" alt="image" src="https://github.com/user-attachments/assets/e85107b6-0c28-40ae-a432-b6381c66dbbf" />

## Key Features

- **One FFT, four views:** 8192 points, Hann-windowed, recomputed every 2048 samples for 75% overlap,
  mapped to 1024 bins on a logarithmic 20 Hz - 20 kHz axis with a -120 dB floor.
  - `SPECTRUM` - live curve, Bezier-interpolated with a moving average so it reads as a shape rather
    than a flickering picket fence.
  - `SPECTRO` - 200 frames of history as a colour-mapped spectrogram, where a resonance that only
    appears on certain notes gives itself away.
  - `3D` - the same history lifted into a waterfall with a depth axis, so a sustained partial becomes
    a ridge you can follow back in time.
  - `WAVEFORM` - plain oscilloscope over a 2048-sample ring buffer.
- **Analog-style VU meters:** stereo level metering on a dBFS scale, with inertia, peak hold and
  colour coding by level.
- **FREEZE:** stops whichever view is on screen without touching the audio thread.
- **Decay presets:** SLOW, MED and FAST set how quickly the display lets go of what it saw.
- **Five themes:** MIDNIGHT, PHOSPHOR, EMBER, VAPOR and ARCTIC, cycled from one button.
- **Supported format:** VST3, Windows x64.

Note: the FFT analyses the left channel. The VU meters are stereo.

## Tech Stack

- **Language:** C++
- **Framework:** [JUCE](https://juce.com/)
- **DSP:** Native JUCE FFT processing and decibel conversion mapping.

## How to Build

If you want to clone this repository and compile the plugin yourself, follow these steps:

1. Ensure you have **JUCE** (Projucer) and your preferred IDE installed (**Visual Studio** for Windows or **Xcode** for macOS).
2. Clone the repository:
   ```bash
   git clone https://github.com/lluisestape-upc/SpectrumAnalyzer.git
3. Open `Spectrum.jucer` using **Projucer**.
4. Click the button to open the project in your IDE.
5. In your IDE, change the build configuration from `Debug` to **`Release`** for optimal performance.
6. Build the solution.
7. You will find the compiled `.vst3` file in the `Builds/` folder.

## Installation (For Users)

1. Download the latest zip from the **Releases** tab of this repository and unzip it.
2. Place the `Spectrum.vst3` folder in your system's VST3 plugin folder:
   - **Windows:** `C:\Program Files\Common Files\VST3`
   - **macOS:** `/Library/Audio/Plug-Ins/VST3`
3. Open your DAW (Ableton, FL Studio, Logic, etc.), rescan your plugins, and load it onto your Master bus or any track.

---

## More Plugins

This plugin is part of the **ESP free plugin collection**.
Download it and find more free audio plugins at:

[esp-plugin-store.vercel.app](https://esp-plugin-store.vercel.app)

## License

GPL v3 -- see [LICENSE](LICENSE). This plugin links the JUCE modules, which are
licensed under AGPLv3, and the VST3 SDK under its GPLv3 option.
