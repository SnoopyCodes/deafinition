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

/*
  ==============================================================================

    This file contains the startup code for a PIP.

  ==============================================================================
*/

#include "MainComponent.h"

class Application    : public juce::JUCEApplication
{
public:
    //==============================================================================
    Application() = default;

    const juce::String getApplicationName() override       { return "SimpleFFTTutorial"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }

    void initialise (const juce::String&) override
    {
        mainWindow.reset (new MainWindow ("SimpleFFTTutorial", new MainComponent, *this));
    }

    void shutdown() override                         { mainWindow = nullptr; }

private:
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (const juce::String& name, juce::Component* c, JUCEApplication& a)
            : DocumentWindow (name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                                .findColour (ResizableWindow::backgroundColourId),
                              juce::DocumentWindow::allButtons),
              app (a)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);

           #if JUCE_ANDROID || JUCE_IOS
            setFullScreen (true);
           #else
            setResizable (true, false);
            setResizeLimits (300, 250, 10000, 10000);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

    private:
        JUCEApplication& app;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (Application)
