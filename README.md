# Software Rendering Pipeline
Graphics rendering pipeline without any external graphical dependencies running fully on CPU.

<p align="center">  
  <img src="doc/dice.gif"> <img src="doc/wall.gif">
</p>

# Controls
- `W/S` - camera movement.
- `Z/C` - model up/down movement.
- `E/Q` - model yaw rotation.
- `R/F` - model pitch rotation.
- `4/5/6/8` - light movement.
- `9/7` - light movement closer/further.

# Screenshots
<p align="center">  
  <img src="doc/1.jpg" width=300px> <img src="doc/2.jpg" width=300px> <img src="doc/3.jpg" width=300px> <img src="doc/4.jpg" width=300px> <img src="doc/5.jpg" width=300px> <img src="doc/6.jpg" width=300px>
</p>

# Features
- You can move around.
- You can rotate a model and move a point light.
- You can load and render `obj` file including materials.
- Phong reflection model.
- Phong shading.
- Texture mapping.
- Normal mapping.
- Z-buffering.
- Render to texture.
- Rendering process emulating standard GPU pipeline.

# Process
There are several stages mimicking standard pipeline.

## Input Assembler
The stage itself does nothing (yet) but it lets you to specify vertex and index buffers.

## Vertex Shader
Vertex shader move vertices from model space to clip (projection) space. My renderer i *right handed* in model space, world space and camera space, and *left handed* in clip space.


# Additional information
The project is built on [tPixelGameEngine](https://github.com/tucna/tPixelGameEngine) for basic window handling. 
