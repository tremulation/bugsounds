#include "MainComponent.h"
#include "SongCodeCompiler.h"


//==============================================================================
MainComponent::MainComponent()
{
    // Set up the songcode editor
    songcodeEditor.setMultiLine(true);
    songcodeEditor.setReturnKeyStartsNewLine(true);
    songcodeEditor.setReadOnly(false);
    songcodeEditor.setScrollbarsShown(true);
    songcodeEditor.setCaretVisible(true);
    songcodeEditor.setPopupMenuEnabled(true);
    songcodeEditor.setText("Enter your songcode here...");
    addAndMakeVisible(songcodeEditor);

    // Set up the echo button
    echoButton.setButtonText("Generate song from code");
    echoButton.onClick = [this] { echoToConsole(); };
    addAndMakeVisible(echoButton);

    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
    
    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    auto area = getLocalBounds().reduced(10);

    //position button at the bottom
    echoButton.setBounds(area.removeFromBottom(30));

    //add space between button and the text editor
    area.removeFromBottom(10);

    //make songCodeEditor fill the remaining space
    songcodeEditor.setBounds(area);

    songcodeEditor.setBounds(getLocalBounds().reduced(10));

}

void MainComponent::echoToConsole()
{
    // Get the text from the editor and print it to the console
    juce::String songcode = songcodeEditor.getText();
    std::vector<SongElement> parsedSong = compileSongcode(songcode.toStdString());

    juce::String output;
    for (size_t i = 0; i < parsedSong.size(); ++i) {
        const auto& element = parsedSong[i];
        output += "Element " + juce::String(i) + ": ";

        if (element.type == SongElement::Type::Note) {
            output += "Note - Start Freq: " + juce::String(element.startFrequency)
                + " Hz, End Freq: " + juce::String(element.endFrequency)
                + " Hz, Duration: " + juce::String(element.duration) + " ms\n";
        }
        else if (element.type == SongElement::Type::Pattern) {
            output += "Pattern - Beats: ";
            for (const auto& beat : element.beatPattern) {
                output += juce::String(static_cast<int>(beat)) + " ";
            }
            output += "\n";
        }
    }

    juce::Logger::writeToLog(output);
}