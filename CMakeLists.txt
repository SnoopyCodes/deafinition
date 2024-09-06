cmake_minimum_required(VERSION 3.15)

# Set the project name
project(deafinition VERSION 1.0)

# Set the path to the JUCE library
set(JUCE_DIR "${CMAKE_SOURCE_DIR}/JUCE")

# Include the JUCE framework
add_subdirectory(${JUCE_DIR})

# Declare the executable and its source files
juce_add_gui_app(deafinition
    PRODUCT_NAME "xd"  # Name of the app
)

# Add the source files
target_sources(deafinition PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
)

# Link the necessary JUCE modules
target_link_libraries(deafinition PRIVATE
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_events
    juce::juce_dsp
    juce::juce_graphics
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
)
