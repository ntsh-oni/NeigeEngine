# Scripting Documentation
NeigeEngine uses **LUA** for the scripting. It also define engine-specific functions described in this documentation.

## Table of contents
1. [Entity](#entity)
2. [Vector](#vector)
3. [Input](#input)
4. [Window](#window)
5. [Time](#time)

## Entity
Functions related to entities and their components.

### Functions
- int **getEntityID()**
- float, float, float **getTransformComponentPosition**(*int entity*)
- float, float, float **getTransformComponentRotation**(*int entity*)
- float, float, float **getTransformComponentScale**(*int entity*)
- **setTransformComponentPosition**(*int entity, float x, float y, float z*)
- **setTransformComponentRotation**(*int entity, float x, float y, float z*)
- **setTransformComponentScale**(*int entity, float x, float y, float z*)

#### int getEntityID()
Takes no parameter and returns the entity calling this script's ID.

#### float, float, float getTransformComponentPosition(int entity)
Takes an entity ID and returns this entity's transform component position (x, y and z).

#### float, float, float getTransformComponentRotation(int entity)
Takes an entity ID and returns this entity's transform component rotation (x, y and z).

#### float, float, float getTransformComponentScale(int entity)
Takes an entity ID and returns this entity's transform component scale (x, y and z).

#### setTransformComponentPosition(int entity, float x, float y, float z)
Takes an entity ID and 3 floats. Set the entity's transform component position.

#### setTransformComponentRotation(int entity, float x, float y, float z)
Takes an entity ID and 3 floats. Set the entity's transform component rotation.

#### setTransformComponentScale(int entity, float x, float y, float z)
Takes an entity ID and 3 floats. Set the entity's transform component scale.

## Vector
Functions to manipulate vectors.

### Functions
- float, float, [float], [float] **normalize**(*float x, float y, [float z], [float w]*)
- float **dot**(*float x1, float y1, [float z1], [float w1], float x2, float y2, [float z2], [float w2]*)
- float, float, float **cross**(*float x1, float y1, float z1, float x2, float y2, float z2*)

#### float, float, [float], [float] normalize(float x, float y, [float z], [float w])
Normalize a vector of 2, 3 or 4 floats and returns a vector of the same size.

#### float dot(float x1, float y1, [float z1], [float w1], float x2, float y2, [float z2], [float w2])
Computes a dot product between **two vectors of the same size** (2, 3 or 4 floats) and returns a float.

#### float, float, float cross(float x1, float y1, float z1, float x2, float y2, float z2)
Computes a cross product between **two vectors of 3 floats** and returns a vector of 3 floats.

## Input
Functions related to keyboard and mouse inputs.

### Functions
- int **getKeyState**(*string key*)
- int **getMouseButtonState**(*string button*)
- double, double **getMousePosition**()
- **setMousePosition**(*double x, double y*)

#### int getKeyState(string key)
Takes the name of the key as a string and returns its state as an integer.
*Key names:*
"a" - "z" : **Letters** from A to Z
"left" : The **left arrow** key
"right" : The **right arrow** key
"up" : The **up arrow** key
"down" : The **down arrow** key
"escape" : The **escape** key
"space" : The **spacebar** key
"shift" The **left shift** key

*Key states:*
0 : The key got **pressed** during this frame
1 : The key is **held** this frame (pressed the previous frame)
2 : The key has been **released** this frame
3 : The key has no state (not pressed, held, nor released)

#### int getMouseButtonState(string button)
Takes the name of the button as a string and returns its state as an integer.
*Button names:*
"left" : The **left click** button
"right" : The **right click** button
"middle" : The **middle** button
"4" : The **4th** button
"5" : The **5th** button
"6" : The **6th** button
"7" : The **7th** button
"8" : The **8th** button

*Button states:*
0 : The key got **pressed** during this frame
1 : The key is **held** this frame (pressed the previous frame)
2 : The key has been **released** this frame
3 : The key has no state (not pressed, held, nor released)

#### double, double getMousePosition()
Returns the x and y position of the mouse.
The position is relative to the window (0, 0 is the window's top left corner).

#### setMousePosition(double x, double y)
Takes two doubles for the x and y position as arguments.
The position is relative to the window (0, 0 is the window's top left corner).

## Window
Functions related to the application's window.

### Functions
- bool **isWindowFullscreen**()
- **showMouseCursor**(*bool show*)
- **toggleFullscreen**()

#### bool isWindowFullscreen()
Returns **true** if the application is in fullscreen, else returns **false**.

#### showMouseCursor(bool show)
Takes a boolean telling if the mouse cursor should be visible or not.

#### toggleFullscreen()
Puts the application in fullscreen mode if it is not and puts the application in windowed mode if it is in fullscreen mode.

##  Time
Functions related to the time.

### Functions
- double **getDeltaTime**()

#### double getDeltaTime()
Takes no argument and returns the delta time as a double.
The delta time represents the time between the previous and the current frame.
