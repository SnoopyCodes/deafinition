#include "MainComponent.h"

MainComponent::MainComponent()
: forwardFFT(fftOrder), spectrogramImage(juce::Image::RGB, 512, 512, true)
{
    setOpaque(true);
    //change depending on tsuff
    setAudioChannels(3, 2);
    
    //figure out what false is
    decibel_slider.setRange(-40, 40, 1);
    decibel_slider.onValueChange = [this] {
        level = juce::Decibels::decibelsToGain((float) decibel_slider.getValue());
    };
    addAndMakeVisible(decibel_slider);

    startTimerHz(60);

    setSize(700, 500);  //honestly no one cares about size
    logAudioDeviceInfo();
}

MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlock, double sampleRate) {
    //80, 8000 so we get like 64000 samples per second
    std::cout << "prepped " << samplesPerBlock << " " << sampleRate << std::endl;
}

void MainComponent::releaseResources() {}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    //pointer to audio in
    //do fft
    // if (bufferToFill.buffer -> getNumChannels() > 0) {
    //     auto* channelData = bufferToFill.buffer -> getReadPointer(0, bufferToFill.startSample);
    //     for (int i = 0; i < bufferToFill.numSamples; i++) {
    //         pushNextSampleIntoFifo(channelData[i]);
    //     }
    // }

    //actual processing
    auto *device = deviceManager.getCurrentAudioDevice();
    auto activeIn = device->getActiveInputChannels();
    auto activeOut = device->getActiveOutputChannels();
    int maxIn = activeIn.getHighestBit() + 1;
    int maxOut = activeOut.getHighestBit() + 1;
    
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
                for (int sample = 0; sample < bufferToFill.numSamples; sample++) {
                    //chat, what are the chances of this working
                    outBuffer[sample] = inBuffer[sample] * level;
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
    //add the slider in the last 1/10th
    decibel_slider.setBoundsRelative(0, .9f, 1, .1);
}

void MainComponent::resized() {
    
}

void MainComponent::timerCallback() {
    if (nextFFTBlockReady) {
        drawNextLineOfSpectrogram();
        nextFFTBlockReady = false;
        repaint();
    }
}

void MainComponent::pushNextSampleIntoFifo(float sample) {
    //if all samples at this time are ready, then say we are ready
    if (fifoIndex == fftSize) {
        if (!nextFFTBlockReady) { //move all data in
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifo[(size_t) fifoIndex++] = sample;
}

void MainComponent::drawNextLineOfSpectrogram() {
    int rightEdge = spectrogramImage.getWidth() - 1;
    int imageHeight = spectrogramImage.getHeight();
    //moving left 1 pixel, because we are adding one pixel columns
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightEdge, imageHeight);
    //fft
    forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
    //find value range to scale rendering
    //got a "Range" class
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2);

    for (int i = 1; i < imageHeight; i++) { //the pixel we on
        //? wtf is happening
        float skewedProportionY = 1.0f - std::exp(std::log((float) i / (float) imageHeight) * 0.2f);
        int fftDataIndex = (size_t) juce::jlimit(0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));

        auto level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

        spectrogramImage.setPixelAt(rightEdge, i, juce::Colour::fromHSV(level, 1.0f, level, 1.0f));
    }
    //also, this function should be running much faster than it actually is..
    //is there any way to decrease audio quality and increase speed?
}