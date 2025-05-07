/*
  ==============================================================================

    SynthVoice.h
    Created: 8 Oct 2024 10:20:01pm
    Author:  Taro

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SynthSound.h"
#include "PipStructs.h"
#include "SongCodeCompiler.h"
#include "HarmonicResonator.h"
#include "Evaluator.h"
#include "Spatializer.h"


class BugsoundsAudioProcessor;

class SynthVoice : public juce::SynthesiserVoice, private juce::Timer {
public:

    //============================== structs =============================================
    struct Click {
        int pos; //position of the next pip in the sequence
        int samplesTilNextClick; //how long until the next pip starts
        int subClicksRemaining;
        float vol;  //from 0 to 1. 
    };


    struct SubClick {
        int samplesRemaining;
        double frequency;
        double phase;
        double maxLevel;
        double curLevel;
        double levelChangePerSample;
    };


    //each voice has a bunch of state variables that are exclusive to that voice.
    //for example: the compiled song, stuff for both layers of click generation, etc.
    struct VoiceState {
        VoiceState() = default;
        VoiceState(VoiceState&&) = default;
        VoiceState& operator=(VoiceState&&) = default;
        VoiceState(const VoiceState&) = delete;
        VoiceState& operator=(const VoiceState&) = delete;
        //song state
        std::vector<SongElement> song;
        int songIndex = 0;
        int samplesRemainingInNote = 0;


        //resonator state
        HarmonicResonator resonator;
        std::vector<SongElement> resSong;
        int resIndex = 0;
        int resSamplesRemainingInNote = 0;
        bool resonatorEnabled = false;
        double resonatorFreq = 0.0f;
        double resonatorFreqDelta = 0.0f;


        //first layer impulse state
        double phase = 0.0;
        double phaseDelta = 0.0;
        double deltaChangePerSample = 0.0;
        double level;
        float timingOffset = 0.0f;

        //second layer impulse state
        double pipPhase = 0.0;
        double pipPhaseDelta = 0.0;
        double pipPhaseDeltaChangePerSample = 0.0;

        //pattern state
        std::vector <uint8_t> beatPattern = { 1 };
        int patternIndex = 0;
        int patternPhaseDivisor = 1;
        int clicksRemainingInBeat = 1;

        std::vector<Click> activeClicks;
        std::vector<SubClick> activeSubClicks;

        //spatialization
        std::unique_ptr<Spatializer> spatializer;
        float distance = 1.0f;
        float angle = 0.0f; //in radians
        float distanceScalar = 1.0f; //scale the max distance param by this to get true distance
        float angleScalar = 0.0f;
        bool hasBeenInitialized = false;    //used for location persistence between playbacks

        //chorus mode state
        enum class VoiceStateState {
            Playing,        //currently making sound
            CoolingDown,    //cooling down in chorus mode
            Dormant         //unused voice, or playback has ended and voices are being culled so we can clear note
        };
        VoiceStateState state = VoiceStateState::Dormant; //zzz zz
        int chorusCooldownSamples = 0;  //time until this voice starts playing again
    };


    std::vector<std::unique_ptr<VoiceState>> voices;

    //================================= Synthvoice default functions ===================================
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNote, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition);
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int /*newPitchWheelValue*/) override { return; }
    void controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/) override { return; }
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;



    //====================================== setters ===================================================

    void setSongScript(juce::ReferenceCountedObjectPtr<ScriptNode> script) { compiledSongScript = script;}
    void setResScript(juce::ReferenceCountedObjectPtr<ScriptNode> script) { compiledResonatorScript = script; }
    void setPipSequence(std::vector<Pip> pips) {
        pipSequence = pips;
        juce::Logger::writeToLog("Pips received. First freq: " + juce::String(pipSequence[0].frequency));
    }
    void setAPVTS(juce::AudioProcessorValueTreeState* apvtsPtr) { apvts = apvtsPtr; }
    void setOwner(BugsoundsAudioProcessor& procPtr) { audioProcessor = &procPtr; }
    void randomizeChorusPositions();
    void beginPeriodicChorusUpdates();
    void pushChorusPositionsToUI();

private:
    //================================= helper functions ==============================================
    void processFirstLayerClicks(VoiceState& voice, double sampleRate, float timingRandomParam,
        const float floorFreq, const float startJitter, const float startFadeout);
    void processSecondLayerClicks(VoiceState& voice);
    float generateAudioOutput(VoiceState& voice, float clickVolumeParam);
    void updateSongProgress(VoiceState& voice);
    void updateResonatorProgress(VoiceState& voice);
    float getBaseAngle(float angleScalar, float maxAngle);
    void reinitializeChorusModeVoice(VoiceState* voice);
    void loadResonatorParams();
    void initializeVoiceState(VoiceState* voice, float vel,
        const std::vector<SongElement>& mainSong,
        const std::vector<SongElement>& resSong,
        bool resonatorEnabled);
    void setupNextResNote(VoiceState& voice, const SongElement& note);
    void setupNextNote(VoiceState& voice, const SongElement& note);
    Click* startNewClick(VoiceState& voice, float clickGenerationFreq, float vol);
    void startNewSubClick(VoiceState& voice, float baseFreq, int samples, float vol);
    void updateVoiceSpatialization(VoiceState* voice, float maxDistance, float stereoSpread);
    void initializeChorusVoice(VoiceState* voice, bool resonatorOn);
    //using this to update the chorus positions continuously whenever something changes
    void timerCallback() override;
    void updateInternalSpatialization(float maxDistance, float stereoSpread);
    

    //=================================== data and references ==============================================
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    BugsoundsAudioProcessor* audioProcessor = nullptr;
    juce::Random rng;

    std::vector<Pip> pipSequence;
    juce::ReferenceCountedObjectPtr<ScriptNode> compiledSongScript;
    juce::ReferenceCountedObjectPtr<ScriptNode> compiledResonatorScript;

    bool isChorusEnabled = false;

    //records the mode that the synth started playing in, so it doesn't change during playback
    int startMode = 0; //0 = mono, 1 = chorus       
    bool playing = false;
    bool stopChorusRefresh = false;

    //used to detect when the positioning parameters have changed
    float lastMaxDistance = -1.0f;
    float lastStereoSpread = -1.0f;
    int lastChorusCount = -1;
    

	//========================= CONSTANTS =========================
	const float timingOffsetMax = 0.50f; //50% of the click length
	const float correlationWeight = 3.0f; //how much the correlation affects the cooldown
	const float boostFactor = 0.2f; //handoff boost factor for correlation algorithm
	const float fillFactor = 0.2f; //how much extra cluster when all voices play
    const float minDist = 5.0f; //prevents voices from spawning on top of the listener
};

