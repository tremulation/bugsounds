# Bug Song Notation

This notation's purpose is to express every possible bug song numerically. 
It's design sacrifices some ease of editing/understanding for flexibility.

The song starts playing when a "MIDI note on" signal is received. If the
note off signal arrives before the song gets to 0, the clicks from the 
synth should die off, based on the length of the last note of the song.

The pitch of the MIDI note is used as an offset for the pitches in the song.
The velocity of the note is used to amplify/quiet the note.


## Components

**Note:** Two numbers back to back, with a comma after the second number.
The first number specifies the target frequency (Hz), and the second specifies
the time it will take to reach that frequency (ms).
	Example: 120 1, 120 1000, 0 500,
	AKA start at 120 hz, remain there for 1 second, and then die off to 0 hz in
	0.5 seconds.
	
**Change Beat Pattern:** Can be used to express different note patterns. Spaces
demarcate beats, the number of sub-beats per beat is expressed as a number from
0 to something really high. Setting it to 0 skips beats.
	EXAMPLES:
	- The default pattern is just one note per beat every beat (which can be
	  expressed like this: pattern(1). 
	- You could also tell it to skip every third beat like this: pattern(1 1 0).
	- If you wanted to play two eighth-notes, and then 3 normal beats, you could 
	  express that like this: pattern(2 1 1 1)
	- the numbers within a pattern block can also be randomized with rand. 

**Loop**: any number of other components, surrounded by "[]" and then a
 length at the end, specifying the number of repetitions. This is basically a
 hack for sticking an LFO into an ADSR envelope.
	Example: 120 1, [120 500, 240 500] 2, 0 500
	AKA start at 120 hz, then interpolate between 120 and 500 hz two times, each time
	taking half a second to go between each time. Then die off in half a second.
	- if the loop iterations isn't a whole number, it gets rounded down.
	- if its <= 0, it gets changed to 1

**Var**: Store a value in a variable and use it later. Variables are shared between 
the two songcode editors, so you can use them for synchronization.
	EXAMPLES:
	-creation: let a = 1000, let b = 1000 * rand(1 10), 
	-use: rand(a b), 1000 a, b 20, let c = a + b 

**Binary Operations**: can be used on numbers, results of rands, variables, etc.
add (+), sub (-), multiply (*), and divide (/). always evaluates down to a number.
	
**Randomize**: Any number from the previous components can be replaced with a 
rand(min, max) which will generate a random float number between the range.
	- works on both numbers in a pair (freq, length), as well as loop
	  iterations, where it will be rounded down
	Example: 120 1, [rand(120 130) 500, rand(240 250) 500] 2, 0 500
	
**Comments**: surround them with dollar signs. EX: $ like this $  
	
## How to Tokenize, Parse and Evaluate

Every time a note on is received, the notation will be parsed and evaluated to
generate a song that the synth will play back.

Here is a toy example: 120 1, rand(120 130) rand(500 600), [pattern(1 rand(0 3) 0 0), rand(120 130) 500, rand(240 250) 500] rand(1 5), 0 500

LEXER PASS:

tok-num(120), tok-num(1), tok-comma, tok-rand, tok-parstart, tok-num(120), tok-num(130), tok-parend...
		
New approach:
do recursive descent parsing.
start by lexing: splitting the string into a list of tokens
	Tok-int 	(1000) has an int value associated with it
	tok-let 	(let)
	tok-id	 	(idname)	has a string value associated with it
	tok-equ  	(=)
	tok-rand 	(rand)
	tok-add  	(+)
	tok-sub  	(-)
	tok-div   	(/)
	tok-mul   	(*)
	tok-barstart 	([)
	tok-barend   	(])
	tok-comma  		(,)
	tok-parstart (   (   )
	tok-parend   (   )   )
	tok-pattern (pattern)

then parse the list of tokens into an AST that we can eval quickly inside the audio thread
running through an AST should generate a list of notes and patterns the synth can play 

there is only one context, all variables are immutable once set, and the resonator and oscillator share a context

Rev 1:

Script 		-> Statement*
Statement 	-> Note COMMA | Pattern COMMA | Declaration COMMA | Loop COMMA
Note		-> AdditiveExpr AdditiveExpr
Pattern		-> PATTERN PARSTART AdditiveExpr* PAREND
Declaration -> LET ID EQUALS AdditiveExpr
Loop 		-> BARSTART Statement* BAREND AdditiveExpr
AdditiveExpr-> MultiplicativeExpr (ADDOPERATOR AdditiveExpr)?
MultiplicativeExpr -> PrimaryExpr (MULTOPERATOR MultiplicativeExpr)?
PrimaryExpr -> INT | ID | Random | PARSTART AdditiveExpr PAREND 
Random      -> RAND PARSTART AdditiveExpr AdditiveExpr PAREND

Helper functions to implement (from CSMC330 project 4)

bool match_token(Vector<Token> toks, TokenType matchType): takes a list of tokens and removes the first token from it, as long as the first token matches the tokenType provided. Returns true if there is a match, and false if there isn't.

std::optional<Token> lookahead(Vector<Token> tokens): returns the first token in the list, or nothing if the list is empty

std::optional<Token> lookahead_many(Vector<Token> tokens, int index): extension of lookahead. Returns token at the given index from the start. lookahead_many toks 0 is equivalent to lookahead toks. 


