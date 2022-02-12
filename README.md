# blinn-phong-shadow-mapping
 A university assignment where I created an exxample environment that is shaded using blinn-phong and uses two-pass shadow mapping to create shadows  

 All code in the renderapp project was written by me, the rest was provided as part of the assignment.

Real-time Graphics Programming for Games 1
5SD805
Olof Wikmark

There are three uniquely textured cubes that rotate and move
over time. They are placed in a scene with a grassy ground and
a stone wall. Everything is shaded using blinn/phong shading and
using shadow mapping to allow the cubes to cast shadows on the
background.

Camera controls are the standard ones for the camera controller
provided by the teacher. WASD moves the camera and holding left
mouse button makes moving the mouse also move the camera.

In the same folder as this readme there is a light.txt file.
This file is read by the program to set light direction and color
for the directional light that lights up the entire scene. Each
line represents a 3d vector with spaces separating the value.
Changing the values changes the lighting of the scene.

The first line represents the light direction vector. This value
is normalized in the shader.

The second line represents the RGB values of the ambient lighting.

The third line represents the RGB values of the diffuse lighting.

The fourth line represents the RGB values of the specular lighting.

The program will not run if there is anything except spaces 
sparating the float values in light.txt or if anything in between
the spaces does not convert to a float using std::to_string().