# Bugsounds

A VST3 software synthesizer that aims to accurately simulate the way insects produce sound (stridulation and tymbalation). It is fit for both sound design and electronic music production. It also features built-in documentation so it isn't too hard to learn.  

It features a unique way to generate clicks, the auditory particles that make up bug song, as well as a notation language that combines envelopes and LFOs, compiled on the fly for each note. This notation language, inspired by Max 8 envelopes, controls both click frequency, and the base frequency of an optional resonator (for bugs like cicadas). Finally, the synth has a chorus mode, to approximate multiple insects playing around the listener.

This project started as a Max 8 patch. You can find that in the "Prototype" directory. There's also a prototype for the resonator, and a prototype for general testing of filters written using the genexpr codebox language.


## Installation

1. [Download juce](https://juce.com/download/) and put its files somewhere convenient. 

2. run the bugsounds.projucer file with projucer.exe in your juce installation directory. It should automatically fill in the dependencies you need to build the project. 

3. press the button at the top of the projucer window to open it in an IDE of your choice.

4. Compile the vst3 version. You'll find the resulting .vst3 file somewhere like ./Builds/VisualStudio2022/x64/Debug/VST3/bugsounds.vst3/Contents/x86_64-win/bugsounds.vst3

5. put it in your computer's vst3 folder (C:\Program Files\Steinberg\VSTPlugins on windows) so your DAW can find it


## License

Everyone who compiles this project owes me one cool bug.
