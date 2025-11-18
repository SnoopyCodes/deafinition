#include "MainComponent.h"

MainComponent::MainComponent()
: fft(fftOrder), spectrogramImage(juce::Image::RGB, 512, 512, true),
window(fftSize, juce::dsp::WindowingFunction<float>::blackman)
{
    setOpaque(true);
    //change depending on tsuff
    setAudioChannels(3, 2);

    auto *device = deviceManager.getCurrentAudioDevice();
    
    //:moyai:
    // decibel_slider.setRange(-40, 40, 1);
    // decibel_slider.onValueChange = [this] {
    //     level = juce::Decibels::decibelsToGain((float) decibel_slider.getValue());
    // };
    // addAndMakeVisible(decibel_slider);

    freopen("audiogram.txt", "r", stdin);
    int N; std::cin >> N;
    boost_gain.resize(N + 2);
    boost_freq.resize(N + 2);
    for (int i = 0; i < N; i++) {
        float freq, db; std::cin >> freq >> db;
        boost_freq[i + 1] = freq;
        boost_gain[i + 1] = juce::Decibels::decibelsToGain(db);
    }
    boost_gain[0] = 1;
    boost_freq[0] = 0;
    boost_gain[N + 1] = 1;
    boost_freq[N + 1] = 25000;
    for (int i = 0; i < N + 2; i++) {
        auto const&[a, b] = std::array<float, 2>{boost_freq[i], boost_gain[i]};
        std::cout << a << " " << std::endl;
    }

    juce::String error = deviceManager.initialise(
        3,  // Input channels
        2,  // Output channels
        nullptr,  // No XML settings
        false,
        "Headphones (2- AirPods Pro)"
    );
    startTimerHz(60);

    const auto& deviceTypes = deviceManager.getAvailableDeviceTypes();

    for (auto* type : deviceTypes) {
        type->scanForDevices();  // Force refresh
        std::cout <<"Device type: " << type->getTypeName() << std::endl;
        
        auto inputNames = type->getDeviceNames(true);
        auto outputNames = type->getDeviceNames(false);
        
        for (auto& name : inputNames) 
            std::cout << "Input: " << name << std::endl;
        for (auto& name : outputNames) 
            std::cout <<"Output: " << name << std::endl;
    }


    setSize(700, 500);  //honestly no one cares about size
    logAudioDeviceInfo();
}

MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlock, double sampleRate) {
    //480, 48000, 100 blocks per second
    // std::cout << "prepped " << samplesPerBlock << " " << sampleRate << std::endl;
    // std::cout << deviceManager.getCurrentAudioDevice()->getName() << std::endl;
}

void MainComponent::releaseResources() {}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    auto *device = deviceManager.getCurrentAudioDevice();
    auto activeIn = device->getActiveInputChannels();
    auto activeOut = device->getActiveOutputChannels();

    int rate = device->getCurrentSampleRate();
    int maxIn = activeIn.getHighestBit() + 1;
    int maxOut = activeOut.getHighestBit() + 1;

    int fifoIndex = 0;
    std::fill(fifo.begin(), fifo.end(), 0);
    std::fill(fftData.begin(), fftData.end(), 0);
    for (int i = 0; i < maxIn; i++) {
        if (!activeIn[i]) { continue; }
        auto *channelData = bufferToFill.buffer->getReadPointer(i, bufferToFill.startSample);
        for (int j = 0; j < bufferToFill.numSamples; j++) {
            fifo[fifoIndex++] = channelData[j];
        }
    }
    std::copy(fifo.begin(), fifo.end(), fftData.begin());
    //do NOT apply a windowing filter, this gets bad.
    // window.multiplyWithWindowingTable(fftData.data(), fftSize);
    //we can now perform an fft
    fft.performRealOnlyForwardTransform(fftData.data());
    int j = 0;
    for (int i = 1; i < fftSize / 2; i++) {
        float freq = i * (rate / fftSize);
        // std::cout << freq << std::endl;
        // std::cout << boost_freq[j + 1] << std::endl;
        while (freq > boost_freq[j + 1]) { j++; }
        float x1 = boost_freq[j];
        float y1 = boost_gain[j];
        float x2 = boost_freq[j + 1];
        float y2 = boost_gain[j + 1];

        float slope = (y2 - y1) / (x2 - x1);
        float inc = slope * (freq - x1) + y1;
        float gain = powf(10.0f, inc / 20.0f);
        
        fftData[2 * i] *= inc;
        fftData[2 * i + 1] *= inc;
    }
    fft.performRealOnlyInverseTransform(fftData.data());
    int data_index = 0;

    for (int channel = 0; channel < maxOut; channel++) {
        if (!activeOut[channel] || maxIn == 0 || channel >= maxIn) {
            bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
        }   else {
            int actualIn = channel % maxIn;
            if (!activeIn[channel]) {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            }   else {
                auto *inBuffer = bufferToFill.buffer->getReadPointer(actualIn, bufferToFill.startSample);
                auto *outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                for (int sample = 0; sample < bufferToFill.numSamples; sample++, data_index++) {
                    if (data_index >= fifo.size()) { data_index -= fifo.size(); }
                    outBuffer[sample] = fftData[data_index];
                }
            }
        }
    }
}

void MainComponent::logAudioDeviceInfo() {
    auto* device = deviceManager.getCurrentAudioDevice();
    if (device != nullptr)
    {
        std::cout << "Audio Device: " << device->getName() << std::endl;
        std::cout << "Sample Rate: " << device->getCurrentSampleRate() << std::endl;
        std::cout << "Buffer Size: " << device->getCurrentBufferSizeSamples() << std::endl;
        std::cout << "Input Channels: " << device->getActiveInputChannels().toString(10) << std::endl;
        std::cout << "Output Channels: " << device->getActiveOutputChannels().toString(10) << std::endl;
    }
    else
    {
        std::cout << "No audio device active!" << std::endl;
    }
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setOpacity(1.0f);
    auto bound_rect = getLocalBounds().toFloat();
    bound_rect.setHeight(bound_rect.getHeight() * 9/10);
    g.drawImage(spectrogramImage, bound_rect);
    decibel_slider.setBoundsRelative(0, .9f, 1, .1);
}

void MainComponent::resized() {}

void MainComponent::timerCallback() {}