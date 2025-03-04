Using a bank of resonators for the harmonic resonator has proven very difficult to get right. Instead, I've written a "harmonic spectral resonator" prototype that I plan on implementing in C++.

- "Harmonic" means it has peaks along each overtone. 
- "Spectral" means that it operates directly on the spectra of the sound, using a fourier transform to get the graph of the spectra, operating on it, and then using the inverse fourier transform to get back to the actual sound. 

Use the code written here: https://github.com/slowslicingaudio/fourier4 as a reference for setting up the forward/inverse FFT, as well as the windows. 

How to implement it in C++:
    1. Adapt the fourier transform windowing setup from this github: https://github.com/slowslicingaudio/fourier4 as a class I can pass blocks to and call process on. 
    2. Find a way to do the frequency shifts described by a song even though the fourier operation works on big windows of samples instead of going sample-by-sample like the current synthVoice resonator code.
    3. Convert the prototype code to a method called process in the resonator class, and create an array of delay objects that is the length of the window (one delay line per frequency bin)

## Spec

data:
    //filter parameters
    int samplerate
    int freq    (20 to 20000, default 440)
    int n       (1 to 8, default 1)
    int q       (0 to 300, default 50)
    float gain    (0 to 50, default 25)
    int delaytime        (in ms, default 10)
    int delayTimeSamples
    int feedbackCoefficient (0 to 1, default .5)

    //FFT parameters
    int windowSize (must be multiple of 2, default 512)
    int overlapFactor (default 4)
    int hopSize (equals windowSize / overlapFactor)

    //state
    int delays[windowSize]
    