/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Evaluator.h"


//==============================================================================
BugsoundsAudioProcessorEditor::BugsoundsAudioProcessorEditor(BugsoundsAudioProcessor& p, float scalingFactor)
    : AudioProcessorEditor(&p), audioProcessor(p), clickSettingsRack(p, *this), frequencyEditor("Frequency Editor", "frequencyEditor", p, *this),
    resonatorEditor("Resonator Editor", "resonatorEditor", p, *this), resonatorKnobRack(p, *this), headerBar(p, *this), pipSequencer(p, *this),
    chorusKnobRack(p, *this), helpCompendium(*this), levelMeter(p), 
    thumbnailCache(1), clickThumbnail(1, formatManager, thumbnailCache)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&myCustomLNF);
    setResizable(true, true);
    setResizeLimits(baseWidth / 2, baseHeight / 2, baseWidth * 3, baseHeight * 3);
    getConstrainer()->setFixedAspectRatio(800.0f/640.0f);
    //set scaling back to what it was before the plugin was minimized
    setSize(baseWidth * scalingFactor, baseHeight * scalingFactor);

                        addAndMakeVisible(headerBar);
    addAndMakeVisible(pipSequencer);       addAndMakeVisible(clickSettingsRack);
    addAndMakeVisible(frequencyEditor);    addAndMakeVisible(resonatorEditor);
    addAndMakeVisible(resonatorKnobRack);  addAndMakeVisible(chorusKnobRack);

    testButton.setButtonText("Compile");
    testButton.onClick = [this] { 
        freqCodeEditorHasChanged(); 
        audioProcessor.setPipSequence(pipSequencer.getPips());
    };
    addAndMakeVisible(testButton);

    addAndMakeVisible(helpCompendium);
    helpCompendium.setVisible(false);

    startTimerHz(24);
    addAndMakeVisible(levelMeter);

    audioProcessor.addChangeListener(this);
	formatManager.registerBasicFormats(); 
}

BugsoundsAudioProcessorEditor::~BugsoundsAudioProcessorEditor() {
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    audioProcessor.removeChangeListener(this);
}

//==============================================================================
void BugsoundsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colour(0xff222222));
}

void BugsoundsAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    int mainWidth = getWidth();
    if (helpCompendium.isVisible()) mainWidth = mainWidth - (mainWidth * (250.f / 800.f));
    const float scalar = (float)getHeight() / (float)baseHeight;
    //save scaling to the plugin processor
    audioProcessor.UIScalingFactor = scalar;

    //------------------- header. 6.25% of total height ------------------- 
    auto headerHeight = (int)(getHeight() * 40.f / 640.f);
    juce::Rectangle<int> headerArea;
    float  guideWidth = helpWidth * scalar;
    if (helpCompendium.isVisible()) {
        headerArea = bounds.removeFromTop(headerHeight).withTrimmedRight(guideWidth);
    } else {
        headerArea = bounds.removeFromTop(headerHeight);
    }

    headerBar.setBounds(headerArea);

    //----------------- Credits overlay. If it's on -----------------------
    if (creditsOverlay) {
        creditsOverlay->scalar = getHeight() / 640.0f;
        float w = 361 * creditsOverlay->scalar;
        float h = 228 * creditsOverlay->scalar;
        creditsOverlay->setSize(w, h);
        auto centered = getLocalBounds().withSizeKeepingCentre(w, h);
        if (helpCompendium.isVisible())
            centered = getLocalBounds().withTrimmedRight(roundToInt(guideWidth)).withSizeKeepingCentre(w, h);
        creditsOverlay->setBounds(centered);
        blocker->setBounds(getLocalBounds());
    }
    
    //----------------------- Pip Sequencer. -------------------------------
    int pipSeqHeight = 189.f * scalar;
    int pipSeqWidth  = 534.f * scalar;
    auto pipSequencerBounds = bounds.removeFromTop(pipSeqHeight);
    auto clickSettingsBounds = pipSequencerBounds.removeFromRight(getWidth() - pipSeqWidth);
    pipSequencer.setBounds(pipSequencerBounds);

    //----------------------- Click Settings. ------------------------------
    if (helpCompendium.isVisible()) clickSettingsBounds.removeFromRight(guideWidth);
    clickSettingsRack.setBounds(clickSettingsBounds);

    //---------------- editors/bottom area setup. --------------------------
    auto bottomArea = bounds;
    bottomArea.removeFromTop(4.f * scalar);  //top padding
    auto editorSide = bottomArea.removeFromLeft(530.f * scalar);
    bottomArea.removeFromLeft(4.f * scalar); //padding between editor and settings
    auto settingsSide = bottomArea; 
    frequencyEditor.setBounds(editorSide.removeFromTop(180.f * scalar));
    resonatorEditor.setBounds(editorSide.removeFromTop(180.f * scalar));
    //add in compile button and decibel meter here later


    //----------------------- Bottom Settings. ------------------------------
    auto settingsModuleHeight = settingsSide.getHeight() / 2.f;
    auto chorusBounds = settingsSide.removeFromTop(settingsModuleHeight);
    if (helpCompendium.isVisible()) chorusBounds.removeFromRight(guideWidth);
    chorusKnobRack.setBounds(chorusBounds);
    if (helpCompendium.isVisible()) settingsSide.removeFromRight(guideWidth);
    resonatorKnobRack.setBounds(settingsSide);

    //-------------------button and vol slider. ----------------------------
    auto compileButtonBounds = editorSide.removeFromLeft(193.f * scalar);
    compileButtonBounds.reduced(4.f * scalar);
    testButton.setBounds(compileButtonBounds);
    levelMeter.setBounds(editorSide.withTrimmedTop(3.f * scalar).withTrimmedBottom(3.f * scalar));

    //help compendium
    if (helpCompendium.isVisible()) {
        auto helpArea = getLocalBounds().removeFromRight(helpWidth * scalar);
        helpCompendium.setBounds(helpArea);
    } 




    //const int padding = 5;
    //const int pipSequencerHeight = 200;
    //const int buttonHeight = 30;
    //const int rackHeight = 110;
    //const int headerHeight = 40;

    //auto fullArea = getLocalBounds();

    //if (helpCompendium.isVisible())
    //{
    //    auto helpArea = fullArea.removeFromRight(helpWidth);
    //    helpCompendium.setBounds(helpArea);
    //}
    //else
    //{
    //    helpCompendium.setBounds(0, 0, 0, 0); // Collapse it completely if hidden
    //}

    //// Now fullArea is only the main part

    //// Header bar at the top
    //headerBar.setBounds(fullArea.removeFromTop(headerHeight));

    //// add padding around edges
    //auto area = fullArea.reduced(padding);

    //// Split into left and right sections
    //auto leftArea = area.removeFromLeft(area.getWidth() * (8.f / 12.f));
    //leftArea.removeFromRight(padding / 2); // tiny extra padding
    //area.removeFromLeft(padding / 2);       // matching padding

    //const int editorHeight = (leftArea.getHeight() - pipSequencerHeight - buttonHeight - padding * 2) / 2;

    //// Pip Sequencer
    //pipSequencer.setBounds(leftArea.removeFromTop(pipSequencerHeight));
    //leftArea.removeFromTop(padding);

    //// Frequency Editor
    //frequencyEditor.setBounds(leftArea.removeFromTop(editorHeight));
    //leftArea.removeFromTop(padding);

    //// Resonator Editor
    //resonatorEditor.setBounds(leftArea.removeFromTop(editorHeight));
    //leftArea.removeFromTop(padding);

    //// Test Button
    //testButton.setBounds(leftArea.removeFromTop(buttonHeight).reduced(padding, 0));

    //// Right side (controls)
    //clickSettingsRack.setBounds(area.removeFromTop(30 + (rackHeight - 30) * 2));
    //resonatorKnobRack.setBounds(area.removeFromTop(30 + (rackHeight - 30) * 2));
    //chorusKnobRack.setBounds(area);
}


void BugsoundsAudioProcessorEditor::freqCodeEditorHasChanged() {
    juce::String freqSongCode = frequencyEditor.getText();
    ErrorInfo errorInfo = {};
	std::map<std::string, float> env;
    //insert dummy midi variable for testing
    env["midiNote"] = 1.f;
    std::vector<SongElement> songElements;

    //compile check freq songcode 
    juce::ReferenceCountedObjectPtr<ScriptNode> parsedSong = generateAST(freqSongCode.toStdString(), &errorInfo);
    if (errorInfo.message != "") goto freqError;
    //need to evaluate so we can report errors that only show up when you eval
    songElements = evaluateAST(parsedSong, &errorInfo, &env);
	if (errorInfo.message != "") goto freqError;


    //compile check res songcode
    if (*audioProcessor.apvts.getRawParameterValue("Resonator On")) {
        juce::String resSongCode = resonatorEditor.getText();
        juce::ReferenceCountedObjectPtr<ScriptNode> parsedResSong = generateAST(resSongCode.toStdString(), &errorInfo);
        if (errorInfo.message != "") goto resError;
		songElements = evaluateAST(parsedResSong, &errorInfo, &env);
        if (errorInfo.message != "") goto resError;
        //set resonator AST/error message
        audioProcessor.setResAST(parsedResSong);
        resonatorEditor.setError(nullptr);
    }
    
	//now we can send the freq song, since res compiled
    frequencyEditor.setError(nullptr);
    audioProcessor.setSongAST(parsedSong);
    return;
    
    //two failure states. mostly the same, besides error message handling
freqError:
    frequencyEditor.setError(&errorInfo);
    audioProcessor.setSongAST(nullptr);
	audioProcessor.setResAST(nullptr);
    return;

resError:
	frequencyEditor.setError(nullptr);
	resonatorEditor.setError(&errorInfo);
	audioProcessor.setSongAST(nullptr);
    audioProcessor.setResAST(nullptr);
	return;
}


void BugsoundsAudioProcessorEditor::toggleHelpCompendium(juce::String pageId) {
    //compute scale only from height (never touch height)
    const float scalar = (float)getHeight() / (float)baseHeight;
    const float helpPx = helpWidth * scalar;
    const int   currentW = getWidth();
    const int   currentH = getHeight();

    if (pageId == "close") {
        goto CloseIt;
    }

    if (helpCompendium.isVisible() && helpCompendium.getPageID() == pageId) {
        // close it
        CloseIt:
        helpCompendium.closeCompendium();
        helpCompendium.setVisible(false);

     
        int newW = currentW - roundToInt(helpPx);
        getConstrainer()->setFixedAspectRatio((float)newW / (float)currentH);
        setSize(newW, currentH);
    } else  {
        // open it (or change page)
        if (!helpCompendium.isVisible())  {
            helpCompendium.setVisible(true);
            helpCompendium.setPage(pageId);

            int newW = currentW + roundToInt(helpPx);
            getConstrainer()->setFixedAspectRatio((float)newW / (float)currentH);
            setSize(newW, currentH);
        } else {
            // visible but a different page -> just swap content
            helpCompendium.setPage(pageId);
        }
    }
    resized();
}



void BugsoundsAudioProcessorEditor::showCreditsWindow() {
    if (creditsOverlay) return;

    //create and add the blocker
    blocker = std::make_unique<ClickBlocker>();
    blocker->onClickOutside = [this]() {
        // remove both components
        removeChildComponent(creditsOverlay.get());
        creditsOverlay.reset();

        removeChildComponent(blocker.get());
        blocker.reset();
    };

    addAndMakeVisible(blocker.get());
    blocker->setBounds(getLocalBounds());

    //draw credits over the top 
    creditsOverlay = std::make_unique<Crebits>();
    addAndMakeVisible(creditsOverlay.get());

    resized();
}


//call in createEditor to set up the thumbnail
void BugsoundsAudioProcessorEditor::setupWaveformThumbnail(const juce::AudioSampleBuffer& waveform, double sampleRate) {
    clickThumbnail.reset(1, sampleRate, waveform.getNumSamples());
}

void BugsoundsAudioProcessorEditor::timerCallback(){
    levelMeter.setLevel(audioProcessor.getRmsValue(0), audioProcessor.getRmsValue(1));
    levelMeter.repaint();
}


void BugsoundsAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &audioProcessor) {
        int numSamples = audioProcessor.lastPreviewBuffer.getNumSamples();
        auto buffer = audioProcessor.lastPreviewBuffer;
        clickThumbnail.reset(1, audioProcessor.getSampleRate(), numSamples);
        clickThumbnail.addBlock(0, buffer, 0, numSamples);
        clickThumbnail.sendChangeMessage();
        repaint();
    }
}


void BugsoundsAudioProcessorEditor::disableResonatorEditor() {
    resonatorEditor.disableEditor();
}


void BugsoundsAudioProcessorEditor::enableResonatorEditor() {
    resonatorEditor.enableEditor();
}