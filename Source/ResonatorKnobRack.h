#pragma once

#include <JuceHeader.h>

// Forward declarations
class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;

//==============================================================================
class PowerButtonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//==============================================================================
class ResonatorKnobRack : public juce::Component {
public:
    ResonatorKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor);
    ~ResonatorKnobRack() override;

    void disableResonatorEditor();
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void initializeKnob(juce::Slider& slider, juce::Label& label,
        const juce::String& labelText,
        const juce::String& paramName,
        std::unique_ptr<SliderAttachment>& attachment);

    // Member variables
    const int margin = 5;

    // --- Top row knobs ---
    // These will control Bandwidth, Gain, and Mix.
    juce::Slider resonatorQKnob, resonatorGainKnob, resonatorMixKnob;
    juce::Label qLabel, gainLabel, mixLabel;

    // --- Bottom row knobs ---
    // These will control Harmonic Emphasis, Overtones, and Drive.
    juce::Slider resonatorHarmonicsKnob, resonatorOvertoneKnob, resonatorDriveKnob;
    juce::Label harmonicsLabel, overtoneLabel, driveLabel;

    PowerButtonLookAndFeel powerButtonLAF;
    std::unique_ptr<juce::ToggleButton> powerButton;

    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> harmonicsAttachment;
    std::unique_ptr<SliderAttachment> overtoneAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<ButtonAttachment> powerButtonAttachment;

    juce::Label titleLabel;

    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;
};
