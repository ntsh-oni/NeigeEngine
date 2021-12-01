# Scripting Documentation - UI
Functions associated with the user interface.

## UI
Prefix: ``ui:``

### Functions
- **number loadSprite(string spritePath)**: Loads a sprite image file. Returns the sprite's id.
- **number loadFont(string fontPath, number height)**: Loads a font from a **TrueType** font file and the height. Returns the font's id.
- **drawSprite(number spriteId, vec2 position, number opacity)**: Draws the sprite during the frame this function is called. The position is relative the window's origin (top left corner).
- **drawText(string text, number fontId, vec2 position, vec4 color)**: Draws the text during the frame this function is called. The position is relative the window's origin (top left corner). The *color* parameter is the RGBA color.
- **drawRectangle(vec2 position, vec2 size, vec4 color)** : Draws a rectangle from *position* to *position + size*. The *color* parameter is the RGBA color.
- **vec2 getSpriteSize(number spriteId)**: Returns the size of the sprite.

[>> Scripting documentation index](../index.md)