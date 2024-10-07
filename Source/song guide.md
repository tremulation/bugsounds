# Bug Song Notation

This notation's purpose is to express every possible bug song numerically. 
It's design sacrifices some ease of editing/understanding for flexibility.

The song starts playing when a "MIDI note on" signal is received. If the
note off signal arrives before the song gets to 0, the clicks from the 
synth should die off, based on the length of the last note of the song.

The pitch of the MIDI note is used as an offset for the pitches in the song.
The velocity of the note is used to amplify/quiet the note.


## Components

**Pair:** Two numbers back to back, with a comma after the second number.
The first number specifies the target frequency (Hz), and the second specifies
the time it will take to reach that frequency (ms).
	Example: 120 1, 120 1000, 0 500,
	AKA start at 120 hz, remain there for 1 second, and then die off to 0 hz in
	0.5 seconds.
	
**Loop**: any number of other components, surrounded by "[]" and then a
 length at the end, specifying the number of repetitions. This is basically a
 hack for sticking an LFO into an ADSR envelope.
	Example: 120 1, [120 500, 240 500] 2, 0 500
	AKA start at 120 hz, then interpolate between 120 and 500 hz two times, each time
	taking half a second to go between each time. Then die off in half a second.
	- if the loop iterations isn't a whole number, it gets rounded down.
	- if its <= 0, it gets changed to 1
	
**Randomize**: Any number from the previous components can be replaced with a 
rand(min, max) which will generate a random float number between the range.
	- works on both numbers in a pair (freq, length), as well as loop
	  iterations, where it will be rounded down
	Example: 120 1, [rand(120 130) 500, rand(240 250) 500] 2, 0 500
	
**Linked Random (lrand)**: Random numbers linked with a character. If two lrands have
the same linking character, they will have the same value. The way this works is the
first time an lrand is computed, it's value is then stored and mapped to the character.
When another lrand is encountered and it has that same character, the second lrands
value will be set to the first value. 
	- Example: lrand(a 140 150) 500, lrand(a) 1000 
     evals to: 144 500, 144 1000
	- you can use an lrand with just a character after the value has already been mapped

**Change Beat Pattern:** Can be used to express different note patterns. Spaces
demarcate beats, the number of sub-beats per beat is expressed as a number from
1 to the max 8, and you can skip beats by replacing the number with "0"
	EXAMPLES:
	- The default pattern is just one note per beat every beat (which can be
	  expressed like this: pattern(1). 
	- You could also tell it to skip every third beat like this: pattern(1 1 0).
	- If you wanted to play two eighth-notes, and then 3 normal beats, you could 
	  express that like this: pattern(2 1 1 1)
	- the numebers within a pattern block can also be randomized. Just like
	  loop repititions there are rules: round down, max 10 min 0.
	
	I'm really not sure if this is a good idea. However, this could be just
	what I need to create that skipping sound that I accidentally found by
	misallocating impulses in the prototype.
	
## How to Tokenize, Parse and Evaluate

Every time a note on is received, the notation will be parsed and evaluated to
generate a song that the synth will play back.

Here is a toy example: 120 1, rand(120 130) rand(500 600), [pattern(1 rand(0 3) 0 0), rand(120 130) 500, rand(240 250) 500] rand(1 5), 0 500

TOKENIZE FIRST PASS, tokenize based on commas, also find loop points and length and move them to their own token.
	0: 120 1
	1: rand(120 130) rand(500 600)
	2: [							<-- Loop start
	3: pattern(1 rand(0 3) 0 0)
	4: rand(120 130) 500
	5: rand(240 250) 500
	6: ] 							<-- Loop end
	7: rand(1 5)					<-- and it's iterations
	8: 0 500
			
TOKENIZE SECOND PASS, expand the loop into more fucking tokens. Resolve the loop iteration rand
here. rand(1 5) 
	0: 120 1
	1: rand(120 130) rand(500 600)
	2: pattern(1 rand(0 3) 0 0)
	3: rand(120 130) 500
	4: rand(240 250) 500
	5: pattern(1 rand(0 3) 0 0)
	6: rand(120 130) 500
	7: rand(240 250) 500
	8: 0 500	
	
TOKENIZE THIRD PASS, resolve the rands from inside of the loop
	0: 120 1
	1: 120.1 599.6			
	2: pattern(1 2.4 0 0)
	3: 120.1 500
	4: 245.9 500
	5: pattern(1 1.01 0 0)
	6: 126.1 500
	7: 241.1 500
	8: 0 500
