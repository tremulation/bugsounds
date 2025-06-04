/*
  ==============================================================================

    ResonatorKnobRack.cpp
    Created: 4 Feb 2025 4:30:21pm
    Author:  Taro

  ==============================================================================
*/

#include "ResonatorKnobRack.h"
#include "PluginEditor.h"  



//==============================================================================
ResonatorKnobRack::ResonatorKnobRack(BugsoundsAudioProcessor& processor, BugsoundsAudioProcessorEditor& editor)
    : audioProcessor(processor), audioEditor(editor)
{
    powerButton = std::make_unique<juce::ToggleButton>("");
    powerButton->setClickingTogglesState(true);
    powerButton->setName("ResonatorPower");
    addAndMakeVisible(powerButton.get());

    //set up the rack's title
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(21.0f).withExtraKerningFactor(.1f));
    titleLabel.setColour(Label::textColourId, Colour::fromString("#000000").withAlpha(1.0f));
    titleLabel.setJustificationType(juce::Justification::left);
    titleLabel.setText("Resonator Settings", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    // --- Top row: Bandwidth, Gain, Mix ---
    initializeKnob(resonatorOvertoneKnob, overtoneLabel, "Overtones", "Resonator Overtone Number", overtoneAttachment);
    initializeKnob(resonatorQKnob, qLabel, "Bandwidth", "Resonator Q", qAttachment);
    initializeKnob(resonatorGainKnob, gainLabel, "Peak Gain", "Resonator Gain", gainAttachment);
    resonatorOvertoneKnob.setValueStyle(" overtones", 0, false);
    resonatorQKnob.setValueStyle(" Hz", 1, false);
    resonatorGainKnob.setValueStyle(" %", 1, false);

    // --- Bottom row: Harmonic Emphasis, Overtones, Drive ---
    initializeKnob(resonatorDecayKnob, oDecayLabel, "Overtone Decay", "Resonator Overtone Decay", oDecayAttachment);
    initializeKnob(resonatorOriginalMixKnob, originalMixLabel, "Original Mix", "Resonator Original Mix", originalMixAttachment);
    resonatorDecayKnob.setValueStyle(" %", 1, true);
    resonatorOriginalMixKnob.setValueStyle(" % ", 1, true);

    powerButtonAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, "Resonator On", *powerButton);

    powerButton->onClick = [this] {
        disableResonatorEditor();
        };

    helpButton = std::make_unique<HelpButton>(
        [this] { audioEditor.toggleHelpCompendium("resonatorSettings"); });
    helpButton->setName("ResonatorHelp");
    addAndMakeVisible(helpButton.get());
}

void ResonatorKnobRack::paint(juce::Graphics& g)
{
    auto  bounds = getLocalBounds();
    float scalar = getHeight() / 189.f;
    auto  headerBounds = bounds.removeFromTop(35.f * scalar);
    auto  bodyBounds = bounds;

    //header
    Colour c1 = Colour::fromString("#818BCA").withAlpha(1.0f);
    Colour c2 = Colour::fromString("#818BCA").withAlpha(1.0f);
    drawUIBlock(g, headerBounds, c1, c2, false, true, scalar);

    //body
    c1 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    c2 = Colour::fromString("#D9D9D9").withAlpha(1.0f);
    drawUIBlock(g, bodyBounds, c1, c2, false, true, scalar);
}


void ResonatorKnobRack::paintOverChildren(juce::Graphics& g) {
    bool resonatorOn = audioProcessor.apvts.getRawParameterValue("Resonator On")->load() > 0.5f;
    if (!resonatorOn) {

        //mask the power button so it doesn't look like it's disabled too
        auto overlayBounds = getLocalBounds().reduced(5);
        auto powerButtonArea = powerButton->getBounds();
        auto originalClip = g.getClipBounds();
        g.excludeClipRegion(powerButtonArea);

        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRect(getLocalBounds().reduced(5));

        //draw the text
        g.setColour(juce::Colours::white);
        g.setFont(24.0f);
        g.drawText("DISABLED", getLocalBounds(), juce::Justification::centred, true);
        repaint();
    }
}

void ResonatorKnobRack::resized()
{
    //lay out header with buttons and title
    auto bounds = getLocalBounds();
    float scalar = getWidth() / 266.0f;
    auto titleHeight = 35.f * scalar;

    auto titleBounds = bounds.removeFromTop(titleHeight);
    auto powerButtonBounds = titleBounds.removeFromLeft(titleHeight);
    powerButton->setBounds(powerButtonBounds);
    titleBounds.removeFromLeft(5.f * scalar);
    titleLabel.setBounds(titleBounds);
    float fontHeight = scalar * 21.f;
    titleLabel.setFont(UIDrawer::getFontInterBold().withHeight(fontHeight).withExtraKerningFactor(.05f));

    auto helpButtonBounds = titleBounds.removeFromRight(titleHeight);
    helpButton->setBounds(helpButtonBounds);


    //divide the remaining area into two rows.
    auto rowHeight = bounds.getHeight() / 2;
    auto topRow = bounds.removeFromTop(rowHeight).translated(0.f, 5.f * scalar);
    auto bottomRow = bounds; // what's left is the bottom row

    //combine knob/label height calculations
    const float knobSize = 60.f * scalar;
    const float labelHeight = 25.f * scalar;
    const float knobTotalHeight = knobSize + labelHeight;
    const float labelFontSize = 17.f * scalar;
    const int randomizeButtonSize = 15 * scalar;

    //loweringOffset shifts knobs downward, and labelSpacing adds extra space between knobs and labels.
    const int loweringOffset = -10 * scalar;
    const int labelSpacing = 10 * scalar;


    // --- Top Row Knobs (3 equally spaced knobs) ---
    float colWTop = topRow.getWidth() / 3.0f;

    auto qKnobArea = topRow.removeFromLeft(colWTop);
    resonatorQKnob.setBounds(qKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    qLabel.setFont(qLabel.getFont().withHeight(labelFontSize));
    qLabel.setBounds(qKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto gainKnobArea = topRow.removeFromLeft(colWTop);
    resonatorGainKnob.setBounds(gainKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    gainLabel.setFont(gainLabel.getFont().withHeight(labelFontSize));
    gainLabel.setBounds(gainKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto overtoneKnobArea = topRow;
    resonatorOvertoneKnob.setBounds(overtoneKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    overtoneLabel.setFont(overtoneLabel.getFont().withHeight(labelFontSize));
    overtoneLabel.setBounds(overtoneKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    // --- Bottom Row Knobs (2 equally spaced knobs) ---
    int bottomRowColumns = 2;
    int bottomRowKnobWidth = bottomRow.getWidth() / bottomRowColumns;

    auto decayKnobArea = bottomRow.removeFromLeft(bottomRowKnobWidth);
    resonatorDecayKnob.setBounds(decayKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    oDecayLabel.setFont(oDecayLabel.getFont().withHeight(labelFontSize));
    oDecayLabel.setBounds(decayKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));

    auto mixKnobArea = bottomRow;
    resonatorOriginalMixKnob.setBounds(mixKnobArea.withSizeKeepingCentre(knobSize, knobSize).translated(0, loweringOffset));
    originalMixLabel.setFont(originalMixLabel.getFont().withHeight(labelFontSize));
    originalMixLabel.setBounds(mixKnobArea.removeFromBottom(labelHeight).translated(0, loweringOffset + labelSpacing));
}


void ResonatorKnobRack::initializeKnob(AnimatedKnobSlider& slider, juce::Label& label, const juce::String& labelText, const juce::String& paramName, std::unique_ptr<SliderAttachment>& attachment) {
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(UIDrawer::getFontInterRegular());
    label.setColour(Label::textColourId, Colours::black);
    label.setJustificationType(juce::Justification::centred);
    slider.updateOriginalLabelText();
    addAndMakeVisible(label);

    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramName, slider);
}


ResonatorKnobRack::~ResonatorKnobRack()
{
    powerButton->setLookAndFeel(nullptr);
}

void ResonatorKnobRack::disableResonatorEditor()
{
    if (powerButton->getToggleState())
        audioEditor.enableResonatorEditor();
    else
        audioEditor.disableResonatorEditor();
}
