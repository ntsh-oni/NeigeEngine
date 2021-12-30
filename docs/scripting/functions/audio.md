# Scripting Documentation - Audio
Functions associated with audio.

## Audio
Prefix: ``audio:``

### Functions
- **number load(string filePath)**: Loads an audio file. Supported types: **Wav** and **Ogg Vorbis**. Returns the sound's id.
- **play(number soundId)**: Plays the sound.
- **stop(number soundId)**: Stops the sound if it is playing.
- **pause(number soundId)**: Pauses the sound if it is playing.
- **bool isPlaying(number soundId)**: Returns true if the sound is currently playing.
- **setGain(number newGain)**: Modifies the sound's gain.
- **setPitch(number newPitch)**: Modifies the sound's pitch.
- **number getSoundCount()**: Returns the number of loaded sounds.

[>> Scripting documentation index](../index.md)