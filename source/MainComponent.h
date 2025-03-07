#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_dsp/juce_dsp.h>;
#include <juce_graphics/juce_graphics.h>
#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

class audio_processor : public juce::AudioIODeviceCallback {
public:
    void audioDeviceIOCallbackWithContext(const float *const* inputChannelData,
                               int totalNumInputChannels,
                               float *const * outputChannelData,
                               int totalNumOutputChannels,
                               int numSamples,
                               const juce::AudioIODeviceCallbackContext &context) override {
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            // Copy input to output (pass-through)
            if (channel < totalNumOutputChannels) {
                std::memcpy(outputChannelData[channel], inputChannelData[channel], sizeof(float) * numSamples);
            }
        }
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {std::cout <<"lets go" << std::endl;}

    void audioDeviceStopped() override {std::cout << "bruh" << std::endl;}
};

class MainComponent :
// public juce::AudioAppComponent,
public juce::Component,
private juce::Timer  {
public:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    MainComponent();
    ~MainComponent() override;

    // void prepareToPlay(int samplesPerBlock, double sampleRate) override;
    // void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    // void releaseResources() override;
    void timerCallback() override;
    void resized() override;
    void logAudioDeviceInfo();

    void pushNextSampleIntoFifo(float);

    void drawNextLineOfSpectrogram();

    void paint(juce::Graphics&) override;

private:
    juce::Image spectrogramImage;
    juce::dsp::FFT forwardFFT;
    juce::Random random;
    juce::AudioDeviceManager deviceManager;


    juce::Slider decibel_slider;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;

    audio_processor processor;

    int fifoIndex = 0;
    float level = 0;
    bool nextFFTBlockReady = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
