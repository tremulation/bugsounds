
## The plan:
1. button pressed -> send song string and pip sequencer list to synthVoice through the thing
    - you can't change these during playback. they're static until a new song is started
    - it would be nice to be able to change the clicks during playback, but I'm not sure how I could accomplish that without locking the audio thread.
2. MIDI note pressed -> song playback begins
    - no playback if songcode doesn't compile in noteStart method
    - else everything is loaded up and the synth goes through the list of notes and patterns the song was compiled into

## SynthesiserVoice
1. startNote()
    - if the "playing" boolean is set to true, then don't start a new song, just ignore this call. 
    - song is compiled into a list of notes and patterns that the synth will be playing back
    - the loudness of the song is scaled to the initial velocity of the midi note
    - the first note is set up with the setupNextNote method and playing is set to true
2. setupNextNote(songElement nextNote): called on every note change
    - if nextNote is a pattern: set up the pattern state (load the pattern from nextNote into synthVoices beatPattern array, and set the pattern state variables to the first element). Once the pattern is set up, advance to the next note in the song, since patterns have no length. Explanation of the pattern state variables:
        - beatPattern: this int array stores the current pattern. The synth will loop through this over and over until the pattern changes again
        - patternPhaseDivisor: this divides the phase of the impulse generator, making it click more frequently. For example, if the patternPhaseDivisor is 4, the synth will click 4 times for every 1 in the song.
        -clicksRemainingInBeat: keeps track of how many subclicks remaining until the 
    - if nextNote is a note: calculate the angleDelta (the starting frequency), and set the amount the frequency will change each sample to reach the ending freuqency in the length of the note in samples
3. renderNextBlock(): this is the function that the sound comes out of. It takes in an audioBuffer and calculates the sound for the synth to generate sample by sample. 
    - for each sample in which the synth is playing:
    - decide if the synth should play a click here. If so: play one (by calling startNewClick), then update the impulse generator state, and the pattern state)
    - render the clicks that are playing right now by calling renderActiveClicks().
    - if this note is over, then either setup the next note, or finish the song.
4. renderActiveClicks(): generates the actual audio signal. Each time startNewClick is called, a new click is added to the array of active clicks. This function manages their state and plays each of them back. 
5. startNewClick(): sets up a new click in the array of playing clicks. It first calculates random offsets for the click's frequency and level, and then adds it to the activeClicks array. 


## Pseudocode for the main rendering function

renderNextBlock()
    for every sample i {
        if the phase equals ((1.0 / patternPhaseDivisor) + timingOffset) 
            {
            add a new click to the activeClicks array (startNewClick)
            set phase to 0
            advance the pattern
            set a new randon timingOffset
            }
        ** Run second-layer click generation here. 
        render the currently-playing clicks (renderActiveClicks)
        add the rendered audio of the clicks to the output buffer
        advance the impulse generator 
        advance the frequency of the currently-playing note 
        if the current note is over
        {
            if the current song is over 
            {
                stop playing
            } else
            {   
                play the next note in the song
            }
        }
    }


## second layer click generation:

split the activeClicks into two arrays: activeClicks and activeSubClicks
activeClicks stores the list of currently-playing clicks 

```c++
struct Click {
    int pos; //position of the next pip in the sequence
    int samplesTilNextClick //how long until the next pip starts
    int subClicksRemaining;
}

//same as the old clicks 
struct SubClick {
    float frequency;
    int samplesRemaining;   
    float phase; 
    float level;
}

std::vector<Click> activeClicks;
std::vector<SubClick> activeSubClicks;
```

in renderNextBlock():

```c++
for (Click c : activeClicks){
    if(c.samplesTilNextClick <= 0 && c.subClicksRemaining > 0) {
        //start a new subClick with startNewSubClick
    }
    //remove any clicks that are finished. A click is finished if there are >= 0 samples remaining, and 0 subclicks remainign
    samplesTilNextClick--;
}
```

