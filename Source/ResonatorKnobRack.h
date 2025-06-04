#pragma once

#include <JuceHeader.h>
#include "ButtonsAndStuff.h"
#include "UIDrawer.h"
#include "AnimatedKnob.h"

// Forward declarations
class BugsoundsAudioProcessor;
class BugsoundsAudioProcessorEditor;


//==============================================================================
class ResonatorKnobRack : public juce::Component, public UIDrawer {
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

    void initializeKnob(AnimatedKnobSlider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment);


    // Member variables
    const int margin = 5;

    // --- Top row knobs ---
    // These will control Bandwidth, Gain, and overtoneNum
    juce::Label qLabel, gainLabel, overtoneLabel, oDecayLabel, originalMixLabel;
    AnimatedKnobSlider resonatorQKnob{ qLabel };
    AnimatedKnobSlider resonatorGainKnob{ gainLabel };
    AnimatedKnobSlider resonatorOriginalMixKnob{ originalMixLabel };
    AnimatedKnobSlider resonatorOvertoneKnob{ overtoneLabel };
    AnimatedKnobSlider resonatorDecayKnob{ oDecayLabel };

    std::unique_ptr<juce::ToggleButton> powerButton;
    std::unique_ptr<HelpButton>         helpButton;

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
