# Scripting Documentation - UI
Functions associated with the user interface.

## UI
Prefix: ``ui:``

### Functions
- **number loadSprite(string spritePath)**: Loads a sprite image file. Returns the sprite's id.
- **number loadFont(string fontPath, number height)**: Loads a font from a **TrueType** font file and the height. Returns the font's id.
- **drawSprite(number spriteIndex, vec2 position)**: Draws the sprite during the frame this function is called. The position is relative the window's origin (top left corner).
- **drawText(string text, number fontIndex, vec3 color, vec2 position)**: Draws the text during the frame this function is called. The position is relative the window's origin (top left corner).

[>> Scripting documentation index](../index.md)