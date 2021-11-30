# Scripting Documentation - UI
Functions associated with the user interface.

## UI
Prefix: ``ui:``

### Functions
- **number loadSprite(string spritePath)**: Loads a sprite image file. Returns the sprite's id.
- **number loadFont(string fontPath, number height)**: Loads a font from a **TrueType** font file and the height. Returns the font's id.
- **drawSprite(number spriteId, vec2 position)**: Draws the sprite during the frame this function is called. The position is relative the window's origin (top left corner).
- **drawText(string text, number fontId, vec2 position, vec3 color)**: Draws the text during the frame this function is called. The position is relative the window's origin (top left corner).
- **drawRectangle(vec2 position, vec2 size, vec3 color)** : Draws a colored rectangle from *position* to *position + size*.
- **vec2 getSpriteSize(number spriteId)**: Returns the size of the sprite.

[>> Scripting documentation index](../index.md)