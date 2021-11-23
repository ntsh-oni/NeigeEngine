# Scripting Documentation
NeigeEngine uses **Lua** for the scripting. It also defines engine-specific functions described in this documentation.

## Table of contents
1. General functions
	- [init](#init)
	- [update](#update)
	- [destroy](#destroy)
2. Types
	- [vector](types/vector.md)
3. Functions
	- [Audio](functions/audio.md)
	- [Camera](functions/camera.md)
	- [Debug](functions/debug.md)
	- [Entity](functions/entity.md)
	- [Input](functions/input.md)
	- [Physics](functions/physics.md)
	- [Time](functions/time.md)
	- [UI](functions/ui.md)
	- [Window](functions/window.md)

## General functions

### init
**init()** is a function that activates once at the entity's creation. It can be used to define variables usable by the rest of the script.

### update
**update()** is a function that activates once per frame. Its number of activations per second heavily depends on the user's hardware and the application. It is recommended to use the **delta time** when the script depends on the number of frames per second.

### destroy
**destroy()** is a function that activates once at the entity's destruction. If multiple entities got destroyed during the same frame, the first entity that got destroyed activates its destroy() function first (**FIFO**).