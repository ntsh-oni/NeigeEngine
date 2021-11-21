# Scripting Documentation - UI
Functions associated with the user interface.

## UI
Prefix: ``ui:``

### Functions
- **number createFont(string fontPath, number height)**: Creates a font from a **TrueType** font and the height. Returns the font's id.
- **drawText(string text, number fontIndex, vec3 color, vec2 position)**: Draws the text during the frame this function is called. The position is relative the window's origin (top left corner).

[>> Scripting documentation index](../index.md)