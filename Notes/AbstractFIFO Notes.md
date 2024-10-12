relaying arrays between different sides of the editor
a bunch of rules for transfering data from the application layer to the ui layer
- no locking the audio thread from the ui. Very bad. 
you need lock free programming.

A FIFO queue is basically a ring buffer. You will never collide if you never read past the write thread.

### juce::AbstractFifo
