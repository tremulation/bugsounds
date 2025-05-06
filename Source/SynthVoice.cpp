/*
  ==============================================================================

    SynthVoice.cpp
    Created: 26 Apr 2025 9:09:13pm
    Author:  Taro

  ==============================================================================
*/

#include "SynthVoice.h"
#include "PluginProcessor.h"

//always true 
//i think
bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound) {
    //if cast fails, then we can't play this sound (since it's the wrong type)
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}


// ================================================================================

void SynthVoice::startNote(int /*midiNote*/, float velocity, juce::SynthesiserSound* /*sound*/, int /*currentPitchWheelPosition*/) {
    isChorusEnabled = apvts->getRawParameterValue("Chorus On")->load();
    startMode = isChorusEnabled ? 1 : 0; //0 = mono, 1 = chorus
    bool resonatorOn = apvts->getRawParameterValue("Resonator On")->load();
    if (startMode == 1) {   //CHORUS MODE

        const int chorusCount = apvts->getRawParameterValue("Chorus Count")->load();
        const float stereoSpread = apvts->getRawParameterValue("Chorus Stereo Spread")->load();
        const float maxDistance = apvts->getRawParameterValue("Chorus Max Distance")->load();

        //make sure we have enough voices already to do playback
        while (voices.size() < chorusCount) voices.push_back(std::make_unique<VoiceState>());

        //VOICE INITIALIZATION
        for (int i = 0; i < chorusCount; ++i) {
            auto& voice = *voices[i];
            initializeChorusVoice(&voice, resonatorOn);
        }

        playing = true;
        stopChorusRefresh = false;
    }
    else {      //MONO MODE
        //compile songs
        ErrorInfo error;
        std::map<std::string, float> sharedEnv;
        std::vector<SongElement> mainSong = evaluateAST(compiledSongScript, &error, &sharedEnv);
        std::vector<SongElement> resSong = {};
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
void SynthVoice::stopNote(float /*velocity*/, bool /*allowTailOff*/) {
    if (startMode == 0) {
        //mono mode, do nothing
        return;
    }
    else {
        //chorus mode, stop restarting voices once they go on cooldown
        stopChorusRefresh = true;
    }
}


//=============================================================================

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {

    // ========================== 1. PRELMIMINARY STUFF ==========================
    if (!playing) return;

    const auto sampleRate = getSampleRate();
    const float timingRandomParam = *apvts->getRawParameterValue("Click Timing Random");
    const float clickVolumeParam  = *apvts->getRawParameterValue("Click Volume");
    const int numChannels  = outputBuffer.getNumChannels();
    const bool resonatorOn = *apvts->getRawParameterValue("Resonator On");

    const float floorFreq    = *apvts->getRawParameterValue("Click Floor Frequency");
    const float startJitter  = *apvts->getRawParameterValue("Click Start Jitter");
    const float startFadeout = *apvts->getRawParameterValue("Click Start Fadeout");

    if (resonatorOn) loadResonatorParams();

    // ======================== 3. COLLECT ACTIVE VOICES ========================
    //single voice for the mono mode, and every active/cooldown voice for the chorus mode
    std::vector<VoiceState*> activeVoices;
    //mono voice:
    if (startMode == 0) activeVoices.push_back(voices[0].get());    //get raw pointer from unique pointer
    //stero voice:
    else {
        const int chorusVoiceNumber = *apvts->getRawParameterValue("Chorus Count");
        //collect the active voices
        timerCallback();
        for (int i = 0; i < chorusVoiceNumber; ++i) {
            activeVoices.push_back(voices[i].get());
            //load the resonator parameters once per block
            if (voices[i]->resonatorEnabled) voices[i]->resonator.loadParams();
        }
        lastChorusCount = chorusVoiceNumber;
    }

    int offCooldownVoices = 0;

    // ========================= 4. PREPARE TEMP BUFFERS =========================
    //these will store the intermediate output of each voice,
    //before spatialization and mixing into the stero output buffer
    std::vector<juce::AudioBuffer<float>> tempBuffers;
    tempBuffers.reserve(activeVoices.size());
    for (size_t voice = 0; voice < activeVoices.size(); voice++) {
        tempBuffers.emplace_back(1, numSamples);
        tempBuffers.back().clear();
    }

    // =========================== 5. PROCESS VOICES =============================
    //loop over each sample, for each voice. write to their temp buffers
    for (size_t v = 0; v < activeVoices.size(); ++v) {

        auto* voice = activeVoices[v];
        if (voice->state == VoiceState::VoiceStateState::CoolingDown) continue;
        else offCooldownVoices++;

        for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
            processFirstLayerClicks(*voice, sampleRate, timingRandomParam, floorFreq, startJitter, startFadeout);
            processSecondLayerClicks(*voice);
            float voiceOutput = generateAudioOutput(*voice, clickVolumeParam); //resonator handled in this function
            updateSongProgress(*voice);  //also handles switch ending song/switching from playing to cooldown
            updateResonatorProgress(*voice);

            //write output to temp buffer
            tempBuffers[v].setSample(0, sampleIdx, voiceOutput);
        }
    }
    // ====================== 6. CHORUS COOLDOWN MANAGEMENT ======================
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
    }
    else {
        //spatialize the output for each voice, and then mix them together to get the output
        for (size_t v = 0; v < activeVoices.size(); v++) {
            VoiceState* voice = activeVoices[v];
            juce::AudioBuffer<float>& voiceBuffer = tempBuffers[v];
            Spatializer& spatializer = *voice->spatializer;
            voice->spatializer->updatePosition(voice->distance, voice->angle);
            spatializer.processBlock(voiceBuffer, outputBuffer, startSample, numSamples);
        }
    }

    // ========================= 7. FINAL STEREO PROCESSING ============================
       //todo maybe later
}


//===============================================================================


void SynthVoice::randomizeChorusPositions(){
    float maxDistance = *apvts->getRawParameterValue("Chorus Max Distance");
    float stereoSpread = *apvts->getRawParameterValue("Chorus Stereo Spread");
    for (auto& voice : voices) {
        //spatialization
        voice->distanceScalar = rng.nextFloat();
        voice->angleScalar = rng.nextFloat() * 2.0f - 1.0f; //from -1 to 1
        voice->distance = minDist + voice->distanceScalar * (maxDistance - minDist);
        voice->angle = getBaseAngle(voice->angleScalar, stereoSpread);
    }
    updateInternalSpatialization(maxDistance, stereoSpread);
}


//===============================================================================


void SynthVoice::beginPeriodicChorusUpdates(){
    startTimerHz(30);   //30 times a second
}


//===============================================================================


void SynthVoice::pushChorusPositionsToUI(){
    std::vector<BugsoundsAudioProcessor::ChorusVoicePosition> positions;
    const int chorusVoiceNumber = (int)*apvts->getRawParameterValue("Chorus Count");
    int count = 0;

    for (size_t v = 0; v < chorusVoiceNumber && v < (int)voices.size(); ++v) {
        if (voices[v] == nullptr) continue;
        auto* voice = voices[v].get();
        if (!voice) continue;

        bool isPlaying = (voice->state == VoiceState::VoiceStateState::Playing);
        positions.push_back({ voice->distance, voice->angle, isPlaying });

        ++count;
    }

    audioProcessor->setChorusVoicePositions(positions);
}



//===============================================================================


//handles generating clicks from the provided song
    //also manages patterns and randomness

//processFirstLayerClicks(*voice, sampleRate, timingRandomParam, floorFreq, startJitter, startFadeout);
void SynthVoice::processFirstLayerClicks(VoiceState & voice, double sampleRate, float timingRandomParam,
    const float floorFreq, const float startJitter, const float startFadeout)
{
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


        //handle starting fadeout and jitter
        const float currentFrequency = voice.phaseDelta * sampleRate;
        const float freqStartRatio = 1.0f - (currentFrequency / floorFreq);

        //calculate new timing offset.
        float randomOffset = (rng.nextFloat() * timingOffsetMax * 2) - timingOffsetMax;
        float offsetScalar = timingRandomParam; //0 to 1
        float fadeScalar = 1.0f;
        //handle start jitter
        if (currentFrequency < floorFreq && floorFreq > 0) {
            offsetScalar = freqStartRatio * startJitter;
            offsetScalar = juce::jmax<float>(offsetScalar, timingRandomParam);
        }
        voice.timingOffset = (randomOffset * offsetScalar) * (1.0f / voice.patternPhaseDivisor);

        //handle perceptual fadeout in dB
        if (floorFreq > 0.0f) {
            float freqRatio = juce::jlimit(0.0f, currentFrequency / floorFreq, 1.0f);
            float maxFadeDb = startFadeout * -60.0f;
            float dbLevel = juce::jmap(freqRatio, maxFadeDb, 0.0f);
            fadeScalar = juce::Decibels::decibelsToGain(dbLevel);
        }

        voice.activeClicks.back().vol = fadeScalar;
    }

    //update core playback state
    voice.phase += voice.phaseDelta;
    voice.phaseDelta += voice.deltaChangePerSample;
    voice.samplesRemainingInNote--;
}


//===============================================================================


//handles generating subclicks from the active clicks in a voice
void SynthVoice::processSecondLayerClicks(VoiceState& voice) {
    for (auto& click : voice.activeClicks) {
        if (click.samplesTilNextClick <= 0 && click.pos < pipSequence.size()) {
            //start a new subclick with the next pip in the sequence
            const Pip& nextPip = pipSequence[click.pos];
            startNewSubClick(voice, nextPip.frequency, nextPip.length, nextPip.level * click.vol);

            //update the click state to the next pip
            click.pos++;
            click.samplesTilNextClick = std::max(nextPip.length - nextPip.tail, 1);
        }
        else {
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


//===============================================================================


float SynthVoice::generateAudioOutput(VoiceState& voice, float clickVolumeParam) {
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
        voice.activeSubClicks.end());

    //if the resonator is enabled for this voice, then process the output through it
    return voice.resonatorEnabled ? voice.resonator.processSample(output, voice.resonatorFreq) : output;
}


//===============================================================================


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
                    const float cooldownMax = apvts->getRawParameterValue("Chorus Cooldown Max")->load();
                    voice.chorusCooldownSamples = rng.nextFloat() * cooldownMax * getSampleRate();
                    voice.state = VoiceState::VoiceStateState::CoolingDown;
                }
                else {
                    //this voice has completed a song, and the synth is done playing
                    //so we will be putting all voices into dormancy. 
                    voice.state = VoiceState::VoiceStateState::Dormant;

                    //once all voices are dormant, we can clear the current note
                    bool allDone = std::all_of(voices.begin(), voices.end(),
                        [](const auto& v) {return v->state == VoiceState::VoiceStateState::Dormant; });

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


//===========================================================================


//updates the current resSong note that a voice is playing.
//  this function is similar to updateSongProgress, but it doesn't handle ending the song
//  if the resonator song ends while the song is still playing, it stays steady at the final
//  freq of the last note
void SynthVoice::updateResonatorProgress(VoiceState& voice) {
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


//===========================================================================


//quickly converts and updates a voices angle/distance scalars into actual distances and angles
void SynthVoice::updateVoiceSpatialization(VoiceState* voice, float maxDistance, float stereoSpread) {
    float maxDistanceScalar = (maxDistance - minDist) / (15.0f - minDist);
    voice->distance = minDist + voice->distanceScalar * (juce::jmin(maxDistance, 15.0f) - minDist) + (15 * maxDistanceScalar);
    voice->distance = juce::jlimit(5.0f, 15.0f, voice->distance);
    voice->angle = getBaseAngle(voice->angleScalar, stereoSpread);
}


//===========================================================================


//for adding new voices, either at startNote, or when the number of chorus voices change
void SynthVoice::initializeChorusVoice(VoiceState* voice, bool resonatorOn){
     float maxDistance = *apvts->getRawParameterValue("Chorus Max Distance");
     float stereoSpread = *apvts->getRawParameterValue("Chorus Stereo Spread");

    //spatialization
     if (!voice->hasBeenInitialized) {
         voice->distanceScalar = rng.nextFloat();
         voice->angleScalar = rng.nextFloat() * 2.0f - 1.0f; //from -1 to 1
         voice->hasBeenInitialized = true;
     }
    updateVoiceSpatialization(voice, maxDistance, stereoSpread);
    voice->spatializer = std::make_unique<Spatializer>();
    voice->spatializer->prepare(voice->distance, voice->angle, { getSampleRate(), 1024, 2 });

    //first cooldown
    //convert the excitation parameter (0 to 1) to a random cooldown. higher excitation -> shorter cooldown
    const float cooldownMax = apvts->getRawParameterValue("Chorus Cooldown Max")->load();
    voice->chorusCooldownSamples = rng.nextFloat() * cooldownMax * getSampleRate();
    voice->state = VoiceState::VoiceStateState::CoolingDown;
    
    //compile the ast so each voice gets its own randomized version of the song
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

    initializeVoiceState(voice, 1, mainSong, resSong, resonatorOn);
}


//===========================================================================

void SynthVoice::timerCallback() {
    if (!*apvts->getRawParameterValue("Chorus On")) return;
    //detect if the spatialization parameters have changed since last block
    float currentMaxDistance = *apvts->getRawParameterValue("Chorus Max Distance");
    float currentStereoSpread = *apvts->getRawParameterValue("Chorus Stereo Spread");
    if (currentMaxDistance != lastMaxDistance || currentStereoSpread != lastStereoSpread) {
        updateInternalSpatialization(currentMaxDistance, currentStereoSpread);
    } 

    //handle voice creation and dormancy
    if (*apvts->getRawParameterValue("Chorus On")) {
        const int chorusCount = *apvts->getRawParameterValue("Chorus Count");
        if (lastChorusCount != chorusCount) {
            //find the difference, the amount of voices we need to add or remove
            int difference = 0;
            if (lastChorusCount < 0) {
                difference = chorusCount;
                lastChorusCount = 0;
            }
            else difference = chorusCount - lastChorusCount;

            if (difference > 0) {
                //add voices
                //if there aren't already enough voices that are dormant, create some new ones
                while (voices.size() < chorusCount) voices.push_back(std::make_unique<VoiceState>());
                bool resonatorOn = apvts->getRawParameterValue("Resonator On")->load();
                for (int i = lastChorusCount; i < chorusCount; i++) {
                    //only reinitialize new voices, not every voice
                    auto& newVoice = *voices[i];
                    initializeChorusVoice(&newVoice, resonatorOn);
                }
            }
            else {
                //remove voices
                for (int idx = chorusCount; idx < (int)voices.size(); ++idx)
                    voices[idx]->state = VoiceState::VoiceStateState::Dormant;
            }
        }
        lastChorusCount = chorusCount;
    }
    pushChorusPositionsToUI();
}


//===========================================================================


void SynthVoice::updateInternalSpatialization(float maxDistance, float stereoSpread){
    if (!apvts->getRawParameterValue("Chorus On")) return;
    const int chorusVoiceNumber = (int)*apvts->getRawParameterValue("Chorus Count");
    int count = 0;
    for (int v = 0; v < voices.size() && count < chorusVoiceNumber; v++) {
        auto* voice = voices[v].get();
        if (!voice) continue;
        updateVoiceSpatialization(voice, maxDistance, stereoSpread);
        count++;
    }
}


//===========================================================================


//translates the angleScalar (-1 to 1) into a real angle in radians
// maps [0 to 1] to [0 to pi]
// maps (0 to -1] to [pi to 2pi]
float SynthVoice::getBaseAngle(float angleScalar, float maxAngle) {
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


//===========================================================================


void SynthVoice::reinitializeChorusModeVoice(VoiceState* voice) {
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


//===========================================================================


void SynthVoice::loadResonatorParams() {
    for (auto& voice : voices) {
        if (voice->resonatorEnabled) {
            voice->resonator.loadParams();
        }
    }
}


//===========================================================================


void SynthVoice::initializeVoiceState(VoiceState* voice, float vel,
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


//===========================================================================


void SynthVoice::setupNextResNote(VoiceState& voice, const SongElement& note) {
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


//===========================================================================


void SynthVoice::setupNextNote(VoiceState& voice, const SongElement& note) {
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


//===========================================================================


void SynthVoice::startNewClick(VoiceState& voice, float clickGenerationFreq) {
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


//===========================================================================


void SynthVoice::startNewSubClick(VoiceState& voice, float baseFreq, int samples, float vol) {
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


//===========================================================================