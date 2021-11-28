# Scripting Documentation - Window
Functions associated with the window.

## Window
Prefix: ``window:``

### Functions
- **bool isFullscreen()**: Returns **true** if the window is in fullscreen, returns **false** if the window is windowed.
- **setFullscreen(bool fullscreen)**: Makes the window fullscreen if the argument is **true**. Makes the window in windowed if the argument is **false**.
- **showMouseCurso(bool show)**: Shows the mouse cursor if the argument is **true**. Hides the mouse cursor if the argument is **false**.
- **vec2 getMousePosition()**: Returns the mouse's position. The position is relative the window's origin (top left corner).
- **setMousePosition(vec2 newPosition)**: Changes the mouse's position. The position is relative the window's origin (top left corner).
- **vec2 getSize()**: Returns the size of the window.
- **setSize(vec2 newSize)**: Changes the size of the window.
- **setTitle(string newTitle)**: Changes the title of the window.
- **setIcon(string newIconPath)**: Changes the icon of the window.
- **close()**: Closes the window.

[>> Scripting documentation index](../index.md)