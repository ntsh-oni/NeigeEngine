
# Scripting Documentation
NeigeEngine uses **Lua** for the scripting. It also defines engine-specific functions described in this documentation.

## Table of contents
1. General functions
	- [init](#init)
	- [update](#update)
2. Specific functions
	- [Entity](#entity)
	- [Vector](#vector)
	- [Input](#input)
	- [Window](#window)
	- [Time](#time)
 
## General functions

### init
**init()** is a function that activates once at the entity's creation. It can be used to define variables usable by the rest of the script.

### update
**update()** is a function that activates once per frame. Its number of activations per second heavily depends on the user's hardware and the application. It is recommended to use the [delta time](#deltatime) when the script depends on the number of frames per second.

### Entity
Functions related to entities and their components.

#### Functions
- int **getEntityID()** : Returns the entity calling this script's ID.
- string **getRenderableComponentModelPath**(*int entity*) : Takes an entity ID and returns this entity's renderable component model path.
- string **getScriptComponentScriptPath**(*int entity*) : Takes an entity ID and returns this entity's script component script path.
- float, float, float **getTransformComponentPosition**(*int entity*) : Takes an entity ID and returns this entity's transform component position (x, y and z).
- float, float, float **getTransformComponentRotation**(*int entity*) : Takes an entity ID and returns this entity's transform component rotation (x, y and z).
- float, float, float **getTransformComponentScale**(*int entity*) : Takes an entity ID and returns this entity's transform component scale (x, y and z).
- **setTransformComponentPosition**(*int entity, float x, float y, float z*) : Takes an entity ID and 3 floats. Set the entity's transform component position.
- **setTransformComponentRotation**(*int entity, float x, float y, float z*) : Takes an entity ID and 3 floats. Set the entity's transform component rotation.
- **setTransformComponentScale**(*int entity, float x, float y, float z*) : Takes an entity ID and 3 floats. Set the entity's transform component scale.

### Vector
Functions to manipulate vectors.

#### Functions
- float, float, [float], [float] **normalize**(*float x, float y, [float z], [float w]*) : Normalize a vector of 2, 3 or 4 floats and returns a vector of the same size.
- float **dot**(*float x1, float y1, [float z1], [float w1], float x2, float y2, [float z2], [float w2]*) : Computes a dot product between **two vectors of the same size** (2, 3 or 4 floats) and returns a float.
- float, float, float **cross**(*float x1, float y1, float z1, float x2, float y2, float z2*) : Computes a cross product between **two vectors of 3 floats** and returns a vector of 3 floats.

### Input
Functions related to keyboard and mouse inputs.

Keyboard keys are referenced by name:
- "a" - "z" : **Letters** from A to Z
- "left" : The **left arrow** key
- "right" : The **right arrow** key
- "up" : The **up arrow** key
- "down" : The **down arrow** key
- "escape" : The **escape** key
- "space" : The **spacebar** key
- "shift" The **left shift** key

Mouse buttons are referenced by name:
- "left" : The **left click** button
- "right" : The **right click** button
- "middle" : The **middle** button
- "4" : The **4th** button
- "5" : The **5th** button
- "6" : The **6th** button
- "7" : The **7th** button
- "8" : The **8th** button

Key/button states are integers:
- 0 : The key/button got **pressed** during this frame
- 1 : The key/button is **held** this frame (pressed the previous frame)
- 2 : The key/button has been **released** this frame
- 3 : The key/button has no state (not pressed, held, nor released)

#### Functions
- int **getKeyState**(*string key*) : Takes the name of the key as a string and returns its state as an integer.
- int **getMouseButtonState**(*string button*) : Takes the name of the button as a string and returns its state as an integer.

### Window
Functions related to the application's window.

#### Functions
- bool **isWindowFullscreen**() : Returns **true** if the application is in fullscreen, else returns **false**.
- **showMouseCursor**(*bool show*) : Takes a boolean telling if the mouse cursor should be visible or not.
- double, double **getMousePosition**() : Returns the x and y position of the mouse. The position is relative to the window (0, 0 is the window's top left corner).
- **setMousePosition**(*double x, double y*) : Takes two doubles for the x and y positions. The position is relative to the window (0, 0 is the window's top left corner).
- **toggleFullscreen**() : Puts the application in fullscreen mode if it is not and puts the application in windowed mode if it is in fullscreen mode.
- int, int **getWindowSize**() : Returns the size of the window (width and height).

###  Time
Functions related to the time.

#### Functions
- <a id="deltatime"></a>double **getDeltaTime**() : Returns the delta time as a double. The delta time represents the time between the previous and the current frame.
