Notes from this tutorial: https://docs.juce.com/master/tutorial_synth_using_midi_input.html
## Main components to a simple synth

**juce::AudioSource:** Basic class for objects that can produce audio themselves. contains the juce::Synthesiser class. 
- has PrepareToPlay and getNextAudioBlock methods.
- two states: prepared and unprepared. 
- when you want to play from an audioSource, you must first make it prepared by calling prepareToPlay. Then you can make repeated calls to prepareToPlay to process the audio data.
- Once playback is finished, call releaseResources to put it back in an unprepared state

**juce::SynthesiserVoice:** renders one of the voices of the synth, mixing it with other sounding voices in a synthesiser object.
- plays a single sound at a time
- I think this is where I should put the code to play back a song, as well as the code for generating clicks from the pip sequencer. 
**juce::SynthesiserSound:** a description of the sound that can be created as a voice. 
- this will probably contain the list of pitches generated from the song

Here's how to set up a midi keyboard inside the synth window:

in mainContentComponent.h:
```c++
juce::MidiKeyboardState keyboardState;
SynthAudioSource synthAudioSource;
juce::MidiKeyboardComponent keyboardComponent
```
initialize the SynthAudioSource and keyboardComponent:
```c++
MainContentComponent() 
	: synthAudioSource (keyboardState),
	  keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
	addAndMakeVisible(keyboardComponent);
	setAudioChannels (0, 2);
	
	setSize(600, 160);
	startTimer(400);
}

```

and then to capture the typing keyboard and use it to play the midi keyboard, we need to wait a couple ms after the keyboard has been initialized:
```c++
void timerCallback() override
{
	keyboardComponent.grabKeyboardFocus();
	stopTimer();
}
```

## AudioAppComponent functions


you're just passing the calls on to the the AudioSource for my synth. (I think this would be pluginProcessor for me, since I'm building a vst plugin instead of a standalone app).
These are "pure virtual functions", whatever that means.

**Pure virtual functions:** they're like abstract functions from parent classes in java. The parent class doesn't come with an implementation, but all child classes that inherit from it must have one. 

```c++
void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
{
	synthAudioSource.prepareToPlay(samplesPerBlockExpected, samplerate);
}

void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override 
{
	synthAudioSource.getNextAudioBlock(bufferToFill);
}

void releaseResources() override
{
	synthAudioSource.releaseResources();
}
```

## The SynthAudioSource Class

The single " : " after the name of the class is I guess the way you do inheritance in c++. Equivalent to an "extends". Why is it also used in the constructor for the midi keyboard state? What does that mean?
```c++
class SynthAudioSource : public juce::AudioSource
{
	public:
		SynthAudioSource( juce::MidiKeyboardState& keyState) : keyboardState (keyState)
		{
					//add some voices to the synth. Determines the amount of polyphony
			for(auto i = 0; i < 4; i++)
				synth.addVoice(new SineWaveVoice());
			//add the sound so the synth knows what sounds it can play
			synth.addSound (new SineWaveSound());
		}

		void setUsingSineWaveSound() {
			synth.clearSounds();
		}

		void prepareToPlay(int samplesPerBlock, double sampleRate) override{
			//synth needs samplerate of audio input
			synth.setCurrentPlaybackSampleRAte(sampleRate);
		}

		void releaseResources() override {}

		void getNextAudioBlock (const juce::audioSourceChannelInfo& bufferToFill) override {
			bufferToFill.clearActiveBufferRegion();
			juce::MidiBuffer incomingMidi;
			//pull buffers of midi data from MidiKeyboardState object
			keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample. bufferToFill.numSamples, true);
			//pass the midi buffers into the synth, so it can use the timestamps for midi-on and midi-off messages to render the voices
			synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
		}
	private:
}
```

## SineWaveSound class

very simple synthesisersound class. Doesn't need to contain any data. It just needs ot report whether this sound should play on particular MIDI channels and specific notes or note ranges on that channel. It just returns true in both cases.
My busounds synth will indeed need to store data (the compiled song that is about to play, etc)

```c++
struct SineWaveSound : public juce::SynthesiserSound
{
	SineWaveSound () {}
	bool appliesToNote (int) override        { return true; }
	bool appliesToChannel (int) override     { return true; }
}
```

## Sine wave voice state
a bit more complex. needs to maintain the **state** of a single voice in the synth. Needs these data members.
```c++
pricate:
	double currentAngle = 0.9, angleDelta = 0.0, double level = 0.0, tailOff = 0.0;
```

## Checking which sound can be played
in SynthesiserVoice. Dynamic cast is being used to check that the type of the sound is correct. 

```c++
bool canPlaySound (juce::SynthesiserSound* sound) override
{
	return dynamic_cast<SineWaveSound*> (sound) != nullptr;
}
```

## Starting a voice

SynthesiserVoice::startNote must be overridden
```c++
void startNote(int midiNotNumber, float velocity, juce::SynthesiserSound*, int  /* currentPitchWheelPosition */) override{
	currentAngle = 0.0;
	level = velocity * 0.15;
	tailOff = 0;

	auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
	auto cyclerPerSample = cyclesPerSecond / getSampleRate();

	angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;

}
```

## Rendering a voice

SynthesiserVoice::renderNextblock must be overridden to generate the audio.
```c++
void renderNextBlock (juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override 
{
	if(angleDelta != 0){
		//key is released. Tail off.
		if(tailOff > 0.0) {
			while (--numSample >= 0){
				auto currentSample = (float) (std::sin(currentAngle) * level * tailOff);
				for(auto i = outputBuffer.getNumChannels(); --i >0;)
					outputBuffer.addSample (i, startSample, currentSample);
	
				currentAngle += angleDelta;
				++startSample;
				//simple exponential decay envelope
				tailOff *= 0.99;
				if(tailOff <= 0.005){
					//tailOff small enough to end current note.
					//reset voice so it can be re-used w/ clearCurrentNote
					clearCurrentNote();
					angleDelta = 0.0;
					break;
				}
			}
		}
	} else {
		//loop for when the note ios being held down.
		//addSample() mixes the current sample with the value already in the buffer at that position 
		//(since other voices might have added stuff)
		while(NumSamples >= 0) {
			auto currentSample = (float) (std::sin(currentAngle) * level);
			for(auto i = outputBuffer.getNumChannels(); --i >= 0)
				outputBuffer.addSample(i, startSample, currentSample);

			currentAngle += angleDelta;
			++startSample;
		}
	}
}
```


## Stopping a voice
with the SynthesiserVoice::stopNote() function. 
Here, the note tail-off is triggered by setting tailOff to 1.0
```c++
void stopNote (float, /*velocity*/, bool allowTailOff) override
{
	if(allowTailOff)
	{
		if(tailOff == 0.0)
		{
			tailOff = 1.0;
		}
	} else {
		clearCurrentNote();
		angleDelta = 0.0;
	}
}

```