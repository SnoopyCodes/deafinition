#include "MainComponent.h"

MainComponent::MainComponent()
: forwardFFT(fftOrder), spectrogramImage(juce::Image::RGB, 512, 512, true)
{
    setOpaque(true);
    setAudioChannels(2, 0);
    
    startTimerHz(60);

    setSize(700, 500);  // Set the initial size of the component
}
MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlock, double sampleRate) {}
void MainComponent::releaseResources() {}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    //pointer to audio in
    if (bufferToFill.buffer -> getNumChannels() > 0) {
        auto* channelData = bufferToFill.buffer -> getReadPointer(0, bufferToFill.startSample);
        for (int i = 0; i < bufferToFill.numSamples; i++) {
            pushNextSampleIntoFifo(channelData[i]);
        }
    }

}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black);
    g.setOpacity(1.0f);
    g.drawImage(spectrogramImage, getLocalBounds().toFloat());
}

void MainComponent::timerCallback() {
    if (nextFFTBlockReady) {
        drawNextLineOfSpectrogram();
        nextFFTBlockReady = false;
        //just like java lol
        repaint();
    }
}

void MainComponent::pushNextSampleIntoFifo(float sample) {
    //if all samples at this time are ready, then say we are ready
    //

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

    //moving left 1 pixel, because we are adding one pixel column
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
}