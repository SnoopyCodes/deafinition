#include "MainComponent.h"

MainComponent::MainComponent() {
    setSize(800, 600);  // Set the initial size of the component
    setAudioChannels(2, 2);
}
MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    filter.setCoefficients(juce::IIRCoefficients::makeAllPass(sampleRate, 1000.0));
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    //pointer to audio in
    auto* channelData = bufferToFill.buffer -> getReadPointer(0);
    if (bufferToFill.buffer ->getNumChannels() > 0) {
        auto* inputChannel = bufferToFill.buffer->getReadPointer(0);
        juce::Logger::outputDebugString("sample: " + juce::String(inputChannel[0]));
    }

}

void MainComponent::releaseResources() {
    //??? what
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::white);  // Fill background with white
    g.setColour(juce::Colours::black);
    g.setFont(20.0f);
    g.drawText("Audio Input/Output active?", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // Handle component resizing here
}
