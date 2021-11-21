# Scripting Documentation - Debug
Functions associated with debug.

## Debug
Prefix: ``debug:``

### Functions
- **number getFrametime()**: Returns the time to make a frame. It is possible to compute the number of frames per second (fps) by calculating ``1000.0 / debug:getFrametime()``.
- **string getModelsList()**: Returns the list of models loaded in memory as a string.
- **string getTexturesList()**: Returns the list of textures loaded in memory as a string.
- **showGraphicsMemoryChunks()**: Prints the content of the graphics memory chunks in the console.

[>> Scripting documentation index](../index.md)