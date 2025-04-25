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
#include "PluginProcessor.h"
#include "SongCodeCompiler.h"
#include "HarmonicResonator.h"
#include "Evaluator.h"
#include "Spatializer.h"


class SynthVoice : public juce::SynthesiserVoice {
public:

    //===========================================================================
        

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

        //===============================================================================

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

    //===============================================================================


    //always true 
    //i think
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        //if cast fails, then we can't play this sound (since it's the wrong type)
        return dynamic_cast<SynthSound*>(sound) != nullptr;
    }

    //===========================================================================

    void startNote(int /*midiNote*/, float velocity, juce::SynthesiserSound* /*sound*/, int /*currentPitchWheelPosition*/) override {
		isChorusEnabled = apvts->getRawParameterValue("Chorus On")->load();
        startMode = isChorusEnabled ? 1 : 0; //0 = mono, 1 = chorus
        bool resonatorOn = apvts->getRawParameterValue("Resonator On")->load();
        if (startMode == 1) {   //CHORUS MODE
            
            const int chorusCount = apvts->getRawParameterValue("Chorus Count")->load();
            const float stereoSpread = apvts->getRawParameterValue("Chorus Stereo Spread")->load();
            const float maxDistance = apvts->getRawParameterValue("Chorus Max Distance")->load();
            const float excitation = apvts->getRawParameterValue("Chorus Excitation")->load();

            //make sure we have enough voices already to do playback
            while (voices.size() < chorusCount) voices.push_back(std::make_unique<VoiceState>());

            //VOICE INITIALIZATION
			for (int i = 0; i < chorusCount; ++i) {
				auto& voice = *voices[i];
				
				//spatialization
                voice.distanceScalar = rng.nextFloat();
				voice.angleScalar = rng.nextFloat() * 2.0f - 1.0f; //from -1 to 1
                voice.distance = voice.distanceScalar * maxDistance;
				voice.angle = getBaseAngle(voice.angleScalar, stereoSpread);
				voice.spatializer = std::make_unique<Spatializer>();
                voice.spatializer->prepare(voice.distance, voice.angle, {getSampleRate(), 1024, 1});

                //cooldowns
				//convert the excitation parameter (0 to 1) to a random cooldown. higher excitation -> shorter cooldown
				//TODO maybe add clustered randomization here for a more natural effect
                voice.chorusCooldownSamples = (1.0f - excitation) * cooldownSecondsMax * getSampleRate();
                voice.chorusCooldownSamples *= rng.nextFloat();
                voice.state = VoiceState::VoiceStateState::CoolingDown;

                //separate voice compilations of the ASTs
                ErrorInfo error;
				std::map<std::string, float> sharedEnv;
                std::vector<SongElement> mainSong = evaluateAST(compiledSongScript, &error, &sharedEnv);
                std::vector<SongElement> resSong = {};
                if (resonatorOn) resSong = evaluateAST(compiledResonatorScript, &error, &sharedEnv);

                if (mainSong.empty()) {
                    clearCurrentNote();
                    playing = false;
                    return;
                }

				initializeVoiceState(&voice, velocity, mainSong, resSong, resonatorOn);
			}
            playing = true;
            stopChorusRefresh = false;
        }
        else {      //MONO MODE
            //compile songs
            ErrorInfo error;
            std::map<std::string, float> sharedEnv;
            std::vector<SongElement> mainSong = evaluateAST(compiledSongScript, &error, &sharedEnv);
            std::vector<SongElement> resSong  = {};
            if (resonatorOn) resSong = evaluateAST(compiledResonatorScript, &error, &sharedEnv);

            if (mainSong.empty()) {
                clearCurrentNote();
                return;
            }

            //create new voice (mono) if needed
            if (voices.empty()) voices.push_back(std::make_unique<VoiceState>());
            auto& voice = *voices[0];

			//reset and initialize
            initializeVoiceState(&voice, velocity, mainSong, resSong, resonatorOn);
            voice.state = VoiceState::VoiceStateState::Playing;
            playing = true;
            stopChorusRefresh = true;
        }
    }

    //===========================================================================

    //once playing is set to false, the synth will stop voices from starting new songs,
    //allowing the current voices to finish. When all voices are done, clearCurrentNote is called.
    void stopNote(float /*velocity*/, bool /*allowTailOff*/) override {
        if (startMode == 0) {  
            //mono mode, do nothing
            return;
        }
        else {
            //chorus mode, stop restarting voices once they go on cooldown
            stopChorusRefresh = true;
        }
    }

    //===========================================================================

    void pitchWheelMoved(int /*newPitchWheelValue*/) override {
        return;
    }

    //===========================================================================

    void controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/) override {
        return;
    }

    //===========================================================================


    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {

        // ========================== 1. PRELMIMINARY STUFF ==========================
        if (!playing) return;

        const auto sampleRate = getSampleRate();
		const float timingRandomParam = *apvts->getRawParameterValue("Click Timing Random");
		const float clickVolumeParam = *apvts->getRawParameterValue("Click Volume");
        const int numChannels = outputBuffer.getNumChannels();

        //detect if the spatialization parameters have changed since last block
        float currentMaxDistance = *apvts->getRawParameterValue("Chorus Max Distance");
        float currentStereoSpread = *apvts->getRawParameterValue("Chorus Stereo Spread");
		if (currentMaxDistance != lastMaxDistance || currentStereoSpread != lastStereoSpread) {
			spatializationParametersChanged(currentMaxDistance, currentStereoSpread);
		}

        // ======================== 2. COLLECT ACTIVE VOICES ========================
		//single voice for the mono mode, and every active/cooldown voice for the chorus mode
        std::vector<VoiceState*> activeVoices;
        //mono voice:
        if (startMode == 0) activeVoices.push_back(voices[0].get());    //get raw pointer from unique pointer
        //stero voice:
        else {
            const int chorusVoiceNumber = *apvts->getRawParameterValue("Chorus Count");
			for (int i = 0; i < chorusVoiceNumber; ++i) {
				activeVoices.push_back(voices[i].get());
                //load the resonator parameters once per block
				if (voices[i]->resonatorEnabled) voices[i]->resonator.loadParams();
			}
        }

        int offCooldownVoices = 0;

		// ========================= 3. PREPARE TEMP BUFFERS =========================
        //these will store the intermediate output of each voice,
        //before spatialization and mixing into the stero output buffer
		std::vector<juce::AudioBuffer<float>> tempBuffers;
        tempBuffers.reserve(activeVoices.size());
		for (size_t voice = 0; voice < activeVoices.size(); voice++) {
			tempBuffers.emplace_back(1, numSamples);
            tempBuffers.back().clear();
		}

		// =========================== 4. PROCESS VOICES =============================
		//loop over each sample, for each voice. write to their temp buffers
        for (size_t v = 0; v < activeVoices.size(); ++v) {

            auto* voice = activeVoices[v];
            if (voice->state == VoiceState::VoiceStateState::CoolingDown) continue;
            else offCooldownVoices++;

            for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx){
                processFirstLayerClicks(*voice, sampleRate, timingRandomParam);
                processSecondLayerClicks(*voice);
				float voiceOutput = generateAudioOutput(*voice, clickVolumeParam); //resonator handled in this function
                updateSongProgress(*voice);  //also handles switch ending song/switching from playing to cooldown
                updateResonatorProgress(*voice);

                //write output to temp buffer
				tempBuffers[v].setSample(0, sampleIdx, voiceOutput);
            }
        }
        // ====================== 5. CHORUS COOLDOWN MANAGEMENT ======================
        //there are three different strats for how a bug decides when it starts singing
        //1. alternation: avoid starting your song when another bug is already singing. avoid interference.
        //2. random: just wait out the variable cooldown. No consideration for other bugs.
        //3. synchrony: try to sing over other bugs to drown them out.

        //we decide which to use based on a correlation parameter. -1:alternation, 0:random 1: synchrony
        //and then linearly interpolate between them
        if (startMode == 1) {
            const int n = activeVoices.size();
            const float correlation = *apvts->getRawParameterValue("Chorus Correlation");

            //the value for completely random correlation. relies completely on the randomly assigned cooldown
            const int randomDecrement = numSamples;
            const float playingProportion = (float)(offCooldownVoices) / (float)(n);
            int finalDecrement = numSamples;

            for (int v = 0; v < n; v++) {
                auto* voice = activeVoices[v];
				if (voice->state == VoiceState::VoiceStateState::Playing || voice->state == VoiceState::VoiceStateState::Dormant) continue;
                if (correlation > 0) {
                    // 1: syncrony: try to sing when other bugs are singing. 
                    //if all voices are playing, then the cooldown is twice as fast. If no voices, then it's normal length
                    float syncronyDecrement = numSamples * (1 + playingProportion * correlationWeight);
                    finalDecrement = static_cast<int>(juce::jmap(correlation, (float)randomDecrement, syncronyDecrement));
                }
                else {
                    // -1: alternation: avoid singing when other bugs are singing
                    //when lots of voices are singing, slow down and wait for it to get quieter (by increasing your cooldown)
                    float baseSlow = 1.0f - playingProportion * correlationWeight;
                    //and when fewer voices are singing, get ready to sing faster (by decreasing the cooldown faster)
                    float handoffBoost = boostFactor * (1.0f - playingProportion);
                    float alternationDecrement = numSamples * juce::jmax(0.0f, baseSlow + handoffBoost);
                    finalDecrement = static_cast<int>(juce::jmap(correlation + 1, alternationDecrement, (float)randomDecrement));
                }

                voice->chorusCooldownSamples -= finalDecrement;
                if (voice->chorusCooldownSamples <= 0 && !stopChorusRefresh) reinitializeChorusModeVoice(voice);
            }
        }

		// ========================= 6. SPATIALIZE OUTPUT ============================
        if (startMode == 0) {
            for (int i = 0; i < numChannels; i++) {
                //write mono output to both channels of the output buffer
                //spatialization isn't applied in mono mode
				auto* outL = outputBuffer.getWritePointer(0) + startSample;
				auto* outR = outputBuffer.getWritePointer(1) + startSample;
				auto* processedOutput = tempBuffers[0].getReadPointer(0);
                for (int sample = 0; sample < numSamples; sample++) {
                    outL[sample] += processedOutput[sample];
                    outR[sample] += processedOutput[sample];
                }
            }
        } else {
            //spatialize the output for each voice, and then mix them together to get the output
			for (size_t v = 0; v < activeVoices.size(); v++) {
				VoiceState* voice = activeVoices[v];
				juce::AudioBuffer<float>& voiceBuffer = tempBuffers[v];
				Spatializer& spatializer = *voice->spatializer;

				spatializer.processBlock(voiceBuffer, outputBuffer, startSample, numSamples);
			}
        }
    }


	//------------------------------------------------------------------------------

    //handles generating clicks from the provided song
    //also manages patterns and randomness
    void processFirstLayerClicks(VoiceState& voice, double sampleRate, float /*timingRandomParam*/) {
		const double threshold = (1.0f / voice.patternPhaseDivisor) + voice.timingOffset;

        if (voice.phase >= threshold) { //GENERATE A CLICK
            //only generate a click if the pattern element is non-zero
            if (voice.beatPattern[voice.patternIndex] != 0) {
                const float baseFrequency = voice.phaseDelta * sampleRate;
                startNewClick(voice, baseFrequency);
            }

            voice.phase = 0.0f;

            //advance pattern state
			if (--voice.clicksRemainingInBeat <= 0) {
				voice.patternIndex = (voice.patternIndex + 1) % voice.beatPattern.size();
				const uint8_t patternValue = voice.beatPattern[voice.patternIndex];

                //handle zero-values as a single subdivision of the pattern
                voice.clicksRemainingInBeat = patternValue == 0 ? 1 : patternValue;
                voice.patternPhaseDivisor = patternValue == 0 ? 1 : patternValue;
			}

            //calculate new timing offset with randomness
            const float randomOffset = (rng.nextFloat() * timingOffsetMax * 2) - timingOffsetMax;
        }

        //update core playback state
        voice.phase += voice.phaseDelta;
        voice.phaseDelta += voice.deltaChangePerSample;
        voice.samplesRemainingInNote--;
    }


    //handles generating subclicks from the active clicks in a voice
    void processSecondLayerClicks(VoiceState& voice) {
        for (auto& click : voice.activeClicks) {
            if (click.samplesTilNextClick <= 0 && click.pos < pipSequence.size()) {
                //start a new subclick with the next pip in the sequence
                const Pip& nextPip = pipSequence[click.pos];
                startNewSubClick(voice, nextPip.frequency, nextPip.length, nextPip.level * click.vol);

                //update the click state to the next pip
                click.pos++;
                click.samplesTilNextClick = std::max(nextPip.length - nextPip.tail, 1);
            } else {
                //count down to next subclick
                click.samplesTilNextClick--;
            }
        }

        //remove finished clicks with an iterator
        voice.activeClicks.erase(
            std::remove_if(voice.activeClicks.begin(), voice.activeClicks.end(),
                [this](const Click& c) { return c.pos >= pipSequence.size(); }),
            voice.activeClicks.end()
        );
    }

    //-----------------------------------------------------------------------------


    //renders the audio output for a single voice
    //It loops through the subclicks and generates the sound for each one.
    float generateAudioOutput(VoiceState& voice, float clickVolumeParam) {
        float output = 0.0f;

		for (auto& subClick : voice.activeSubClicks) {
            //calc the raw sine oscillator value
			float oscValue = std::sin(subClick.phase * juce::MathConstants<float>::twoPi);
			subClick.phase += subClick.frequency / getSampleRate();
			subClick.phase = std::fmod(subClick.phase, 1.0f);

            //calculate the attack/decay envelope
            if (subClick.curLevel >= subClick.maxLevel) {
                //start decay phase
                const int decaySamples = subClick.samplesRemaining;
                if (decaySamples > 0) {
					subClick.levelChangePerSample = -subClick.maxLevel / decaySamples;
                }
            }

            //update the state
            subClick.curLevel += subClick.levelChangePerSample;
			subClick.samplesRemaining--;

            //add this subclick to the output
            float volumeGain = juce::Decibels::decibelsToGain(clickVolumeParam);
			output += oscValue * subClick.curLevel * volumeGain;
		}

        //remove finished subclicks with an iterator
        voice.activeSubClicks.erase(
            std::remove_if(voice.activeSubClicks.begin(), voice.activeSubClicks.end(),
                [](const SubClick& sc) { return sc.samplesRemaining <= 0; }),
            voice.activeSubClicks.end() );

        //if the resonator is enabled for this voice, then process the output through it
        return voice.resonatorEnabled ? voice.resonator.processSample(output, voice.resonatorFreq) : output;
    }


    //-----------------------------------------------------------------------------


    //updates the current song note that a voice is playing. 
    //Also handles ending playback depending on the mode
    //in mono mode, playback ends when the one voice is done playing
    //in chorus mode, playback ends when ALL voices are done, AND we have already received a noteOff
    void SynthVoice::updateSongProgress(VoiceState& voice) {
        if (voice.samplesRemainingInNote <= 0) {    
            //we've reached the end of a note
            if (++voice.songIndex >= voice.song.size()) {
                //we've reached the end of the song
                if (startMode == 0) { 
                    //mono mode
                    clearCurrentNote();
                    playing = false;
                }
                else {
                    //chorus mode
                    if (!stopChorusRefresh) {
                        //this voice has completed a song, but we aren't done singing yet.
                        //so we need to put this voice into cooldown
                        const float excitation = apvts->getRawParameterValue("Chorus Excitation")->load();
                        voice.chorusCooldownSamples = (1.0f - excitation) * cooldownSecondsMax * getSampleRate();
                        voice.chorusCooldownSamples *= rng.nextFloat();
                        voice.state = VoiceState::VoiceStateState::CoolingDown;
                    }
                    else {
						//this voice has completed a song, and the synth is done playing
						//so we will be putting all voices into dormancy. 
                        voice.state = VoiceState::VoiceStateState::Dormant;

                        //once all voices are dormant, we can clear the current note
						bool allDone = std::all_of(voices.begin(), voices.end(),
							[](const auto& v) {return v->state == VoiceState::VoiceStateState::Dormant;});

                        if (allDone) {
                            clearCurrentNote();
                            playing = false;
                        }
                    }
                }
            }
            else {
                //we HAVENT reached the end of the current song. Just dispatch the next note
                setupNextNote(voice, voice.song[voice.songIndex]);
            }
        }
    }


    //updates the current resSong note that a voice is playing.
    //  this function is similar to updateSongProgress, but it doesn't handle ending the song
    //  if the resonator song ends while the song is still playing, it stays steady at the final
    //  freq of the last note
    void updateResonatorProgress(VoiceState& voice) {
        if (voice.resonatorEnabled) {
            voice.resonatorFreq += voice.resonatorFreqDelta;

            if (--voice.resSamplesRemainingInNote <= 0) {
                if (++voice.resIndex >= voice.resSong.size()) {
                    // Loop resonator song
                    voice.resIndex = 0;
                }
                setupNextResNote(voice, voice.resSong[voice.resIndex]);
            }
        }
    }


    //-----------------------------------------------------------------------------


    //when the spatialization parameters change, all voices need to be updated
	void spatializationParametersChanged(float maxDistance, float stereoSpread) {
        //update the spatialization trackers
		lastMaxDistance = maxDistance;
		lastStereoSpread = stereoSpread;

        //propogate the changes to the spatializers in all voices
        for (size_t v = 0; v < voices.size(); ++v) {
            auto* voice = voices[v].get();
            if (voice == nullptr) continue;
			if (voice->state == VoiceState::VoiceStateState::Dormant) continue;

            voice->distance = voice->distanceScalar * maxDistance;
            voice->angle = getBaseAngle(voice->angleScalar, stereoSpread);

            voice->spatializer->updatePosition(voice->distance, voice->angle);
        }
	}


    //translates the angleScalar (-1 to 1) into a real angle in radians
    // maps [0 to 1] to [0 to pi]
    // maps (0 to -1] to [pi to 2pi]
    float getBaseAngle(float angleScalar, float maxAngle) {
        using R = juce::MathConstants<float>;
        //compute half of the allowed angular spread
        float halfSpan = maxAngle * (R::pi / 2.0f);

        //choose front vs back center
        float center = (angleScalar >= 0.0f)
            ? (R::pi / 2.0f)          // front:  90
            : (3.0f * R::pi / 2.0f);  // back:  270

        //use the absolute scalar for symmetry
        float scalar = (angleScalar >= 0.0f) ? angleScalar : -angleScalar;

        //map [0..1] -> [-halfSpan..+halfSpan]
        float offset = (scalar - 0.5f) * 2.0f * halfSpan;

        return center + offset;
    }


    //-----------------------------------------------------------------------------


    void reinitializeChorusModeVoice(VoiceState* voice) {
        //separate voice compilations of the ASTs
        ErrorInfo error;
        std::map<std::string, float> sharedEnv;
        std::vector<SongElement> mainSong = evaluateAST(compiledSongScript, &error, &sharedEnv);
        std::vector<SongElement> resSong = {};
        bool resonatorOn = apvts->getRawParameterValue("Resonator On")->load();
        if (resonatorOn) resSong = evaluateAST(compiledResonatorScript, &error, &sharedEnv);

        if (mainSong.empty()) {
            clearCurrentNote();
            playing = false;
            return;
        }

        //TODO idk what I'd pass in for velocity here
        initializeVoiceState(voice, 1, mainSong, resSong, resonatorOn);
        voice->state = VoiceState::VoiceStateState::Playing;
        voice->chorusCooldownSamples = 0;
    }


    //-----------------------------------------------------------------------------

    void setSongScript(juce::ReferenceCountedObjectPtr<ScriptNode> script) {
        compiledSongScript = script;
    }

    void setResScript(juce::ReferenceCountedObjectPtr<ScriptNode> script) {
        compiledResonatorScript = script;
    }

    void setPipSequence(std::vector<Pip> pips) {
        pipSequence = pips;
        juce::Logger::writeToLog("Pips received. First freq: " + juce::String(pipSequence[0].frequency));
    }

    void setAPVTS(juce::AudioProcessorValueTreeState* apvtsPtr) {
        apvts = apvtsPtr;
    }

	void loadResonatorParams() {
		for (auto& voice : voices) {
			if (voice->resonatorEnabled) {
				voice->resonator.loadParams();
			}
		}
	}

private:
    juce::Random rng;
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    std::vector<Pip> pipSequence;

    juce::ReferenceCountedObjectPtr<ScriptNode> compiledSongScript;
    juce::ReferenceCountedObjectPtr<ScriptNode> compiledResonatorScript;


	//========================= CONSTANTS =========================

	const float timingOffsetMax = 0.50f; //50% of the click length
    const float cooldownSecondsMax = 10.0f;
	const float correlationWeight = 3.0f; //how much the correlation affects the cooldown
	const float boostFactor = 0.2f; //handoff boost factor for correlation algorithm
	const float fillFactor = 0.2f; //how much extra cluster when all voices play

    bool isChorusEnabled = false;
    //records the mode that the synth started playing in, so it doesn't change during playback
	int startMode = 0; //0 = mono, 1 = chorus       
    bool playing = false;
    bool stopChorusRefresh = false;


    //used to detect when the positioning parameters have changed
    float lastMaxDistance = -1.0f;
    float lastStereoSpread = -1.0f;




    //===============================================================================

    void initializeVoiceState(VoiceState* voice, float vel,
        const std::vector<SongElement>& mainSong,
        const std::vector<SongElement>& resSong,
        bool resonatorEnabled)
    {
        // Manual state reset
        voice->song.clear();
        voice->songIndex = 0;
        voice->samplesRemainingInNote = 0;

        // Reset impulse state
        voice->phase = 0.0;
        voice->phaseDelta = 0.0;
        voice->deltaChangePerSample = 0.0;
        voice->level = vel * 0.15f;

        // Reset pattern state
        voice->beatPattern = { 1 };
        voice->patternIndex = 0;
        voice->patternPhaseDivisor = 1;
        voice->clicksRemainingInBeat = 1;

        // Clear clicks
        voice->activeClicks.clear();
        voice->activeSubClicks.clear();

        // Main song setup
        voice->song = mainSong;
        if (!voice->song.empty() && voice->song[0].type != SongElement::Type::Pattern) {
            voice->song.insert(voice->song.begin(), SongElement{ std::vector<uint8_t>{1} });
        }

        // Initialize first element
        if (!voice->song.empty()) {
            setupNextNote(*voice, voice->song[0]);
        }

        // Resonator setup
        voice->resonatorEnabled = resonatorEnabled;
        if (voice->resonatorEnabled) {
            voice->resSong = resSong;
            voice->resonator.reset(); // Reset internal DSP state
            voice->resonator.setAPVTS(apvts);
            voice->resonator.prepareToPlay(getSampleRate());
            if (!resSong.empty()) {
                setupNextResNote(*voice, resSong[0]);
            }
        }
    }

    //===============================================================================

    void setupNextResNote(VoiceState& voice, const SongElement& note) {
        if (note.type == SongElement::Type::Pattern) {
            //ignore. patterns don't work with the resonator

            voice.resIndex++;
            setupNextResNote(voice, voice.resSong[voice.resIndex]);
        }
        else {
            auto startingFreq = note.startFrequency;
            auto endingFreq = note.endFrequency;
            auto noteLengthInSamples = (note.duration / 1000.0) * getSampleRate();
			voice.resonatorFreq = startingFreq;
			voice.resonatorFreqDelta = (endingFreq - startingFreq) / noteLengthInSamples;
			voice.resSamplesRemainingInNote = (int)noteLengthInSamples;
        }
    }


    void setupNextNote(VoiceState& voice, const SongElement& note) {
        if (note.type == SongElement::Type::Pattern) {
            voice.beatPattern = note.beatPattern;
            voice.patternIndex = 0;

            if (voice.beatPattern[0] == 0) {
                //0 means a skipped click, which should take as much time as a single click
                voice.clicksRemainingInBeat = 1;
                voice.patternPhaseDivisor = 1;
            }
            else {
                voice.clicksRemainingInBeat = voice.beatPattern[0];
                voice.patternPhaseDivisor = voice.beatPattern[0];
            }

            voice.phase = 0.0f;
            //advance to next note, since patterns are 0-length
            voice.songIndex++;
            setupNextNote(voice, voice.song[voice.songIndex]);
        }
        else {
            // calculate how much the angleDelta will have to increase/decrease by
            // to hit the end frequency in exactly curElement->duration ms.
            const double startingPhaseChange = note.startFrequency / getSampleRate();
            const double endingPhaseChange = note.endFrequency / getSampleRate();
            const double noteLengthInSamples = (note.duration / 1000) * getSampleRate();

            voice.phaseDelta = startingPhaseChange;
            voice.samplesRemainingInNote = static_cast<int>(noteLengthInSamples);
            voice.deltaChangePerSample = (endingPhaseChange - startingPhaseChange) / noteLengthInSamples;
        }
    }


    //===============================================================================


    void startNewClick(VoiceState& voice, float clickGenerationFreq) {
        Click newClick = {};
        //create first subclick
        struct Pip firstPip = pipSequence[0];
        startNewSubClick(voice, firstPip.frequency, firstPip.length, firstPip.level);

        //now set up the click state
        newClick.pos = 1;   //already started first pip, go to second
        newClick.samplesTilNextClick = firstPip.length - firstPip.tail;
        if (newClick.samplesTilNextClick <= 0) {
            newClick.samplesTilNextClick = 1;
        }
        newClick.vol = 1.0;

        voice.activeClicks.push_back(newClick);
    }


    void startNewSubClick(VoiceState& voice, float baseFreq, int samples, float vol) {
        SubClick newClick = {};

        //frequency randomization
        const float freqRandomness = *apvts->getRawParameterValue("Click Pitch Random");
        const float freqOffset = (rng.nextFloat() * 2.0f - 1.0f) * freqRandomness;
        const float freqMultiplier = std::pow(2.0f, freqOffset);

        //attack/decay timing
        const float attackRatio = *apvts->getRawParameterValue("Click Atack Decay Ratio");
        const int attackSamples = std::round(attackRatio * samples);

        //configure subclick
        newClick.frequency = baseFreq * freqMultiplier;
        newClick.samplesRemaining = samples;
        newClick.phase = 0.0f;
        newClick.maxLevel = vol;
        newClick.curLevel = 0.0f;
        newClick.levelChangePerSample = vol / static_cast<double>(attackSamples);

        voice.activeSubClicks.push_back(newClick);
    }
};

