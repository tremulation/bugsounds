#Help: Chorus Mode
##Introduction
Chorus mode simulates standing in the middle of a swarm of insects. It takes the song described by the rest of the UI, and plays it dynamically through a swarm of individual insects. 

To play in chorus mode, activate it by pressing the power button to the left of "Chorus Mode" and then press and hold any midi note. As you hold the note, voices will interactively start and stop songs until the note is released. when the note is released, voices will stop coming off cooldown, allowing playing voices to finish.

All parameters, including those from the Subclick Sequencer, can be changed during playback.

##Controls
- Count: The number of voices in the swarm. 1 to 10. 
- Randomize Button: The 'R' button to the right of the position display. Randomizes voice positions. 
- Spread: the stereo width of the swarm. 
- Distance: How far voices are from the listener.
- Cooldown: Cooldowns are generated randomly, and this value specifies the max possible cooldown. 0 to 24 seconds.
- Correlation: Controls how individual voices decide when to come off cooldown. from -1 to 1.
	* -1 : voices alternate
	* 0  : voices wait their random cooldown
	* 1  : voices try to synchronize
	values between -1 and 1 blend these behaviors.
	
	
	

#Help: Resonator Settings

##Introduction
Some insects posess resonant air cavities, such as cicadas. These cavities change size to resonate at different frequencies with the main song. This synthesizer uses several resonators placed at increasing overtones of the fundamental resonant pitch specified by the resonator editor. 

To use the resonator, first turn it on in Resonator Settings. Once it's on, you can use the Resonator Editor on the left to describe how it will behave throughout the song in the resonator editor. 

##Controls
- Bandwidth: How wide the resonant peaks are. Wider peaks will cause a larger band of frequencies to resonate. 
- Peak Gain: How tall the peaks are, or how much resonance to apply.
- Overtones: The number of peaks to place.
- Overtone Decay: how much to decrease the loudness of successive overtones. .5 is no decay, values < .5 cause decay, and values > .5 cause amplification of successive peaks.
- Original Mix: How much of the original un-resonated signal to include in the output.




#Help: Click Settings

##Introduction
Click Settings allows you to edit the properties of individual clicks, as well as song-wide properties. 

##Controls
- Vol: how loud each click is. 
- Time  Rand: how much randomness to inject into click timing
- Pitch Rand: how much randomness to inject into click pitch
- A/D  ratio: what percentage of the subclicks are rising, and what percent is falling. For example, an A/D ratio of .2 will mean all subclicks increase to full volume in 20% of their length, and then fall back to 0 volume in the remaining 80%.
The next three parameters tie the current volume of a song to the speed at which clicks are being generated. This prevents initial clicks from being unnaturally loud at the start of songs. 
- Min Vol Freq: The click frequency at which the minimum volume is reached. 
- Max Vol Freq: The click frequency at which the maximum volume is reached.
- Freq Fadeout: the minimum volume (the volume at Min Vol Freq).





#Help Subclick Sequencer

##Introduction
This UI element describes the individual clicks that this synthesizer generates as a series of extremely short "subclicks" that, when played together, sound to the listener like one single click. 

##Controls
- Subclick bars: Each subclick is represented by a single bar in the series. 
- Mode Selectors: Each subclick has a Frequency, Length, Overlap, and Level value. You can choose which value you are currently editing by clicking on the appropriate tab. Attributes:
	- Frequency: what pitch the subclick vibrates at. in Hz and kHz.
	- Length: how long the subclick lasts.  in microseconds and miliseconds.
	- Overlap: how soon after this subclick begins that the next one does. us and ms again.
	- Level: the relative intensity of the subclick.
- Modifying a Subclick: Select the subclick by clicking on it, and then drag it up or down to change the value of the current attribute. Double click to enter an exact value.
- Adding a Subclick: Click the plus button at the end of the series to add one. 
- Removing a Subclick: Click on a bar to select it and press delete or backspace. 
- Preview: Plays a single example click. 

##Tips
- You might find it easier to first describe the full song in the frequency editor, and then add, modify, or remove subclicks once you have a full song to work with. 
- If you have a lot of subclicks, you might have to scroll to see the plus button again.
- setting an overlap for subclicks at the end doesn't do anything
- clicking in the preview often goes away when you play the full song.

##Explanation
The approach of treating each click as a series of subclicks is meant to mimic the various methods of insect sound reproduction. Most biological methods involve quickly scraping a series or chitinous ridges, or buckling a series of flexibile ribs. In both cases, each ridge or rib has a sound associated with it. A single scrape along the ridge produces a single click, made up of several subclicks played very close together. This scraping or buckling is repeated at great speed, producing continuous notes. 

This synthesizer was made as a demonstration that the subclick -> click -> continuous note model can be used to accurately simulate two different types of rhythmic bug sound production: stridulation, and tymbaling. 





#Help: Frequency Editor

##Introduction
Uses a notation language called clickCode (like sheet music for bugs) to describe a bug song. It is designed to capture all possible insect songs. The frequency editor is compiled first, and then the resonator editor is compiled, using any variables set in the frequency editor. Every element is separated from the next by a comma. 

All clickCode songs are compiled into a list of notes and patterns. to compile the code in both resonators, hit the "Play song from code" button at the bottom of the synth. 

##Syntax
- Note: a pair of values of the form: "frequency(Hz) length(ms)", separated by commas. The frequency specifies the ending frequency, so each note starts at the last note's ending frequency. At the start of a song, the frequency is 0. 
	EX: 120 1000, 0 1000: go from 0 clicks a second to 120 in 1 second, and then go from 120 clicks to 0 in 1 second. 

- Beat Pattern: pattern(x y z). Change the beat allocation pattern.	
	EX: 120 1, [120 500, 240 500] 2, 0 500
	- The default pattern is just one note per beat every beat (which can be
	  expressed like this: pattern(1). 
	- You could also tell it to skip every third beat like this: pattern(1 1 0).
	- If you wanted to play two eighth-notes, and then 3 normal beats, you could 
	  express that like this: pattern(2 1 1 1)
	- the numbers within a pattern block can also be randomized with rand. 

- Loop [note1 note2 note3] iterations. Any number of components, surrounded by a "[]" followed by a number of iterations at the end. 
	- loops with fractions are rounded down. 

- Var: store a value in a variable and use it later. Variables are shared between the two clickCode editors, so you can use them for synchronization
	EX: 
	- creation: let a = 1000 * (rand 1 10), let b = 1000 
	- use : rand(a b) 1000, 100 a, b 20, let c = a + b
	
- Binary Operators:can be used on numbers, results of rands, variables, etc. add (+), sub (-), multiply (*), and divide (/). 
	- always evaluates down to a single number.
	
- Randomize: Any number from the previous components can be replaced with a rand(min max) which will generate a random number between the range
	- works on both numbers in a pair (freq, length), as well as loop iterations, where it will be rounded down

- Comments: surround them with dollar signs. EX: $ like this $  

##Tips

- use zero-length notes to skip the smooth frequency transition from one note to another.
- patterns are the most important part of making a song sound "buglike". 
- The loops are basically a hack for sticking an LFO into an envelope




#Help: Resonator Editor

##Introduction
Whereas the Frequency editor holds clickCode that describes the way that the frequency of click generation changes, the resonator editor holds code to describe the way the resonant pitch changes. 

The frequency editor is compiled first, and then the resonator editor is. Variables created in the frequency editor are shared with the resonator, for synchronization purposes. 

the resonator editor shares all syntax with the frequency editor, however pattern elements are ignored.


