#ifndef GRAPHICS_3D_H
#define GRAPHICS_3D_H

#include  <iostream>
#include    <vector>
#include      <list>
#include <algorithm>

#include "olcPixelGameEngine.h"

#include   "vec3d.h"
#include  "mat4x4.h"

// ============================================================

/* Render modes defined here.
 *
 * Note that depending on the rendering type the painters algorithm and/or a depth buffer is
 * applied. The painters algorithm is used in all grey filled and wire frame modes,
 * the depth buffer is used for all textured modes.
 */
#define RM_UNKNOWN         -1
#define RM_INVISIBLE        0    // Useful to keep wireframe object alive, but not to render (for instance bounding boxes)
#define RM_GREYFILLED       1    // Grey coloured without wire frame drawing
#define RM_GREYFILLED_PLUS  2    //               with     "     "      "
#define RM_WIREFRAME        3    // No texturing, no grey colouring, only wire frame drawing
#define RM_WIREFRAME_RGB    4    // Like RM_WIREFRAME, but using the colour specified in the triangle (instead of RM_FRAMECOL_PGE
#define RM_TEXTURED         5    // Textured      without wire frame drawing
#define RM_TEXTURED_PLUS    6    //               with     "     "      "

// colour for wireframe drawing
#define RM_FRAMECOL_CGE     FG_WHITE    // consoleGameEngine
#define RM_FRAMECOL_PGE     olc::WHITE  // pixelGameEngine

// ============================================================

extern short glbRenderMode;  // default initialized to RM_GREYFILLED_PLUS
extern float *pDepthBuffer;

struct renderColour {
    // pixelGameEngine: rgb values for the colour
    int r, g, b; // could also be short: values between 0 and 255 (including)

    int renderMode;
};

struct triangle {
    vec3d p[3];    // ... is a grouping of three points together
    vec2d t[3];    // ... if textured also three texture coordinates

    // pixelGameEngine: rgb values for the colour
    int r, g, b; // could also be short: values between 0 and 255 (including)

    int renderMode;

    olc::Sprite *ptrSprite;
};

// initialize a depthbuffer with the screen size as passed in the parameters.
void InitDepthBuffer( int nScreenW, int nScreenH );
// this is a clear screen, but then scoped to the size as specified
// (x1, y1) is upper left corner of viewport, (x2, y2) is lower right corner
void ClearDepthBuffer( int nScreenW, int x1, int y1, int x2, int y2 );

// A camera is defined by its location and orientation (both in world space).
// Since the pitch, yaw and roll determine the orientation they are stored in the camera as well.
// The resulting projection and view matrices are part of the camera structure.
// For practical reasons also the view port information is stored in the camera.
class camera {
public:
    std::string sCameraName;

    vec3d vPosition;  // represents the position of the camera in 3D space
    vec3d vPrevPos;   // the previous position can be used for collision detection
                      // below vectors represent the coordinate system for the camera
    vec3d vLookDir;   // typically this will be a unit vector in the direction the camera points
    vec3d vUp;        //                                         the up direction of the camera
    vec3d vRight;

    float fCameraPitch;   // define the rotations around the camera axes
    float fCameraYaw;
    float fCameraRoll;

    float fNearPlane, fFarPlane;

    int   nViewPortX1, nViewPortX2, nViewPortWidth;    // define the view port for this camera
    int   nViewPortY1, nViewPortY2, nViewPortHeight;

    mat4x4 matView;   // view matrix for this camera - calculated using point-at & look-at matrix
    mat4x4 matProj;   // projection matrix for the view port with this camera

    olc::PixelGameEngine *gfxEngine = nullptr;

    // In the GetColour a minimum and maximum RGB-value is used. Theoretically these values are in [0, 255].
    // To get a better visibility, you can set other minimum and maximum RGB values for the
    // grey colouring.
    // If minVal and maxVal don't comply with 0 <= minVal < maxVal <= 255, then nothing is changed.
    void SetRGBrange( int minVal = 0, int maxVal = 255 );

    // Initializes the camera using the parameters. Calculates camera's projection matrix as well.
    // The rotation angles fCameraPitch, fCameraYaw and fCameraRoll are set to 0.0f.
    // the points (x1, y1) and (x2, y2) are top left resp. bottom righ corner of the viewport (in screen coordinates)
    void InitCamera( olc::PixelGameEngine *engine, std::string name, int x1, int y1, int x2, int y2,
                         float fieldOfViewInDegrees = 90.0f, float nearPlaneDistance = 0.1f, float farPlaneDistance = 1000.0f );

    // update the camera's projection matrix with the parameters provided
    void UpdateCamera( float fieldOfViewInDegrees, float nearPlaneDistance, float farPlaneDistance );

    // Kind of clear screen for the viewport associated with the camera
    void ClearCameraViewPort( bool viewPortBorder = false );

    // Whenever the fCameraPitch, -Yaw and/or -Roll are changed, this function can be called to recalculate both
    // the coordinate system of the camera and its view matrix
    void RecalculateCamera();

protected:
    // Performs a transform from triIn to triOut, using transformation matrix trfMatrix.
    // The col and sym values of the triangle are propagated.
    // Note: This method is declared static since it called by static method Tri_WorldTransform()
    static void Tri_Transform( triangle &triIn, mat4x4 trfMatrix, triangle &triOut );

public:
    // Performs "world transformation" (the transform from model to world space) on the input triangle triIn,
    // using worldMatrix. The transformed triangle is passed in triOut.
    // Note 1: Each model in the world potentially needs its own worldMatrix.
    // Note 2: This method is declared static since it is not depending on any specific camera
    static void Tri_WorldTransform( triangle &triIn, mat4x4 &worldMatrix, triangle &triOut );

protected:
    // Performs view transformation (from world to view space) on the input triangle triIn, using viewMatrix.
    // The transformed triangle is passed in triOut
    // Every camera needs its own view matrix.
    void Tri_ViewTransform( triangle &triIn, mat4x4 &viewMatrix, triangle &triOut );
    // Performs projection transformation (from view space to projection space) on the input triangle triIn,
    // using projectionMatrix. The transformed triangle is passed in triOut
    // Every camera has it's own projection matrix.
    void Tri_ProjectTransform( triangle &triIn, mat4x4 &projectionMatrix, triangle &triOut );

public:
    // Scales the coordinates of TriIn into the camera's viewport. The scaled triangle is passed in triOut
    void Tri_ScaleIntoCameraView( triangle &triIn, triangle &triOut );

    // Performs culling, view transform and projection transform on the triangle inputTri. Because of clipping
    // against the near plane, the result can be 0, 1 or 2 triangles, that are added to vecOfTris
    void CullViewAndProjectTriangle( triangle &inputTri, std::vector<triangle> &vecOfTris, vec3d vLightDir = { 1.0f, 1.0f, 1.0f } );

    // Performs the rasterizing and drawing of all the triangles in the vector trisToRaster,
    // and leaves the result in trisToRender
    void RasterizeTriangles( std::vector<triangle> &trisToRaster, std::vector<triangle> &trisToRender );

private:
    // GetColour stuff:
    // In the pixelGameEngine, the primary colours are defined in constants for Red, Green, Blue in intensity ranging 0 - 255.
    //     for instance: GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
    // In between them variants are possible by varying the second parameter of the pixel-triplet.
    // The following two functions use this feature.
    short minRGBvalue, maxRGBvalue;

    // copies all colour info from triIn to triOut
    // Note: This method is declared static since it called by static method Tri_WorldTransform()
    static void Tri_PropagateColourInfo( triangle triIn, triangle &triOut );

    // This function translates a variable a with value in a range between a_min and a_max into
    // a corresponding (i.e. proportional) value in the range b_min to b_max.
    int VaryShade( float a, float a_min, float a_max, int b_min, int b_max );

    // This function translates a variable a with value in a range between a_min and a_max into
    // a corresponding (i.e. proportional) result in the range b_max to b_min.
    // Thus if a == a_min, then result == b_max !!
    int VaryShadeReversed( float a, float a_min, float a_max, int b_min, int b_max );

    // This function is a variant of GetColour(). It uses less black and white shades and thus makes
    // the shades more distinguishable. I noticed that so many black or near black shades were rendered
    // that I often felt like the regular GetColour() wasn't working at all :)
    void GetColour2( float lum, triangle &tri );

    // Clipping function, returns the number of triangles that are created by it.
    // inputs:   plane_p, plane_n --> the plane equation parameters (a point in the plane and the normal vector to the plane)
    //           in_tri           --> the triangle to be clipped
    // outputs:  out_tri1 and out_tri2 (either neither, or the first, or both triangles will be useful)
    int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2);
};

// This function is a variant of the clipping algorithm. It calculates the intersection line segment between a triangle
// and a plane. It returns 0 if no intersection was found, 1 otherwise. Returns -1 upon error.
// inputs:   plane_p, plane_n --> the plane equation parameters (a point in the plane and the normal vector to the plane)
//           in_tri           --> the triangle to be checked against the plane
// outputs:  out_point1 and out_point2 (either neither, or both points will be useful)
int Triangle_IntersectPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, vec3d &out_point1, vec3d &out_point2 );

#endif // GRAPHICS_3D_H
