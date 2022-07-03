# MatrixTransformationDemo
A little demo program to manipulate a cube in 3D space while seeing the effect on the associated transformation matrix.



MatrixTransformDemo

Joseph21 - november 29, 2021



Purpose
=======
This demo shows the effect on

 * scaling factors (in x, y, z dimension)
 * rotation angles (around x, y and z axes)
 * translation offsets (in x, y, z directions)

on a transformation matrix. It also demonstrates this effect by rendering a simple 1x1x1 cube using that transformation matrix as the world matrix.

New in the november version is the ability to change your Field of View (FoV), near plane (fNear) and far plane (fFar) values that are input to the projection matrix in use. This enables you to experiment with different values for these parameters.

Package
=======
The following files are provided with this package:
 * grapics_3D.h and .cpp
 * mat4x4.h and .cpp
 * vec3d.h and .cpp
 * main.cpp

You must provide the header olcPixelGameEngine.h yourself, it is needed but not included in the package.

For the november version only 
 * main.cpp
 * graphics_3D.h and .cpp
are changed. The others sources are not altered.

Installation
============
Just put all the files in the same directory, and bind them into one project with your IDE. This shouldn't be too hard to compile. Don't forget to provide the olcPixelGameEngine.h.

User interface
==============
You can change the scaling, rotation and translation values using the arrow keys:

 * left  - decrease value
 * right - increase value
 * up    - set value to 1.0f
 * down  - set value to 0.0f

Up and down keys can be used to quickly reset to default values. Note that a scaling of 0.0f will make the cube infinitely small. Default for scaling is 1.0f.

By using any of the activation keys you select wich value to change:

 * Q, W, E - scale factors (for x, y, z respectively)
 * A, S, D - rotation angles (idem)
 * Z, X, C - translation offsets (idem)

Example - holding the S key down and pushing the left arrow key will decrease the rotation angle around the y axis. Holding the C key while pushing the up arrow will set the translation offset in the z-direction to 1.0f.

For the FoV, fNear and fFar you can use the V, N and F keys respectively, in combination with the + and - keys from the numeric keypad.

Have fun with it.
