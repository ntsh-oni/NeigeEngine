# Scripting Documentation - Input
Functions associated with inputs.

Keyboard keys are referenced by name:
- **"a"** - **"z"**: Letters from A to Z.
- **"left"**: The left arrow key.
- **"right"**: The right arrow key.
- **"up"**: The up arrow key.
- **"down"**: The down arrow key.
- **"escape"**: The escape key.
- **"space"**: The spacebar key.
- **"shift"**: The left shift key.

Mouse buttons are referenced by name:
- **"left"**: The left click button.
- **"right"**: The right click button.
- **"middle"**: The middle click button.
- **"4"**: The 4th button.
- **"5"**: The 5th button.
- **"6"**: The 6th button.
- **"7"**: The 7th button.
- **"8"**: The 8th button.

## InputState
The InputState enum has 4 values :
- NONE = 0 (InputState.NONE)
- PRESSED = 1 (InputState.PRESSED)
- HELD = 2 (InputState.HELD)
- RELEASED = 3 (InputState.RELEASED)

## Input
Prefix: ``input:``

### Functions
-**InputState getKeyState(string key)**: Returns the state of the keyboard's key.
-**InputState getMouseButtonState(string button)**: Returns the state of the mouse's button.

[>> Scripting documentation index](../index.md)