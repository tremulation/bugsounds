#pragma once

#include <JuceHeader.h>
#include "PowerButtonLAF.h"

// Forward declarations
class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;


//==============================================================================
class ResonatorKnobRack : public juce::Component {
public:
    ResonatorKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor);
    ~ResonatorKnobRack() override;

    void disableResonatorEditor();
    void resized() override;
    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;


private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    //void paintOverChildren(juce::Graphics& g) override;

    void initializeKnob(juce::Slider& slider, juce::Label& label,
        const juce::String& labelText,
        const juce::String& paramName,
        std::unique_ptr<SliderAttachment>& attachment);

    // Member variables
    const int margin = 5;

    // --- Top row knobs ---
    // These will control Bandwidth, Gain, and overtoneNum
    juce::Slider resonatorQKnob, resonatorGainKnob, resonatorOvertoneKnob, resonatorDecayKnob, resonatorOriginalMixKnob;
    juce::Label qLabel, gainLabel, mixLabel, overtoneLabel, oDecayLabel, originalMixLabel;

    PowerButtonLookAndFeel powerButtonLAF;
    std::unique_ptr<juce::ToggleButton> powerButton;

    std::unique_ptr<SliderAttachment> overtoneAttachment;
    std::unique_ptr<SliderAttachment> qAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> oDecayAttachment;
    std::unique_ptr<SliderAttachment> originalMixAttachment;
    std::unique_ptr<ButtonAttachment> powerButtonAttachment;

    juce::Label titleLabel;

    BugsoundsAudioProcessor& audioProcessor;
    BugsoundsAudioProcessorEditor& audioEditor;
};
