DOOM
====

This is an attempt to get a version of DOOM very close to the original running under any system with X11. The primary
purpose of this port is to allow for a very clean and simple source to compile with Emscripten to non-web or node.js
targets (i.e., TurboSphere).

This version currently has a dummied out audio. No work has yet been done to abstract the X11 bindings.
It also has the Z_Malloc from PrBoom.

Due to the original makefiles being a million years old, I added a quick SCons build set.
