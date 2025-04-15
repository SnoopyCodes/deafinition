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

class MainComponent :
public juce::AudioAppComponent,
private juce::Timer  {
public:
    static constexpr int fftOrder = 12;
    static constexpr int fftSize = 1 << fftOrder;
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlock, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void timerCallback() override;
    void resized() override;
    void logAudioDeviceInfo();

    void pushNextSampleIntoFifo(float);

    void drawNextLineOfSpectrogram();

    void paint(juce::Graphics&) override;

private:
    juce::Image spectrogramImage;
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    juce::Random random;

    juce::Slider decibel_slider;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;

    std::vector<float> boost_gain;
    std::vector<float> boost_freq;

    float level = 0;
    bool nextFFTBlockReady = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
