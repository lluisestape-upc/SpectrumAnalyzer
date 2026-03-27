# 🎛️ SpectrumAnalyzer

A real-time Audio Spectrum Analyzer plugin (VST3/Standalone) built with C++ and JUCE. Features a smooth Bézier-interpolated FFT curve, analog-style VU meters, and a stereo panning indicator. Designed for high performance and accurate visual feedback.

<img width="1362" height="794" alt="image" src="https://github.com/user-attachments/assets/e85107b6-0c28-40ae-a432-b6381c66dbbf" />

## ✨ Key Features

* 📈 **Smooth FFT Spectrum:** Real-time frequency analysis utilizing Bézier curve interpolation and moving average smoothing for an organic, jitter-free visual experience.
* 🎚️ **Analog-Style VU Meters:** Stereo level metering in a logarithmic scale (dBFS). Features visual inertia (smooth falloff ballistics) and dynamic color coding.
* ⚖️ **Stereo Panning Indicator:** Real-time visual indicator to monitor your L/R stereo balance and signal width.
* ⚡ **High Performance:** Optimized 60 FPS rendering designed to have a minimal CPU footprint and accurate visual feedback.
* 🔌 **Supported Formats:** VST3 and Standalone desktop application.

## 🛠️ Tech Stack

* **Language:** C++
* **Framework:** [JUCE](https://juce.com/)
* **DSP:** Native JUCE FFT processing and decibel conversion mapping.

## 🚀 How to Build

If you want to clone this repository and compile the plugin yourself, follow these steps:

1. Ensure you have **JUCE** (Projucer) and your preferred IDE installed (**Visual Studio** for Windows or **Xcode** for macOS).
2. Clone the repository:
   ```bash
   git clone [https://github.com/lluisestape-upc/SpectrumAnalyzer.git](https://github.com/lluisestape-upc/SpectrumAnalyzer.git)
3. Open the `.jucer` file using **Projucer**.
4. Click the button to open the project in your IDE.
5. In your IDE, change the build configuration from `Debug` to **`Release`** for optimal performance.
6. Build the solution.
7. You will find the compiled `.vst3` file in the `Builds/` folder.

## 🎧 Installation (For Users)

1. Download the latest `.vst3` file from the **Releases** tab of this repository.
2. Place the file in your system's VST3 plugin folder:
   * **Windows:** `C:\Program Files\Common Files\VST3`
   * **macOS:** `/Library/Audio/Plug-Ins/VST3`
3. Open your DAW (Ableton, FL Studio, Logic, etc.), rescan your plugins, and load it onto your Master bus or any track.
