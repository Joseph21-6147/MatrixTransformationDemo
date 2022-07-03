#include "graphics_3D.h"

//// To prevent all kinds of include problems I redefined some constants from olcConsoleGameEngine.h here.
//// I need these constants because the functions GetColour() depend on them.
//
//#define PIXEL_SOLID             0x2588
//#define PIXEL_THREEQUARTERS     0x2593
//#define PIXEL_HALF              0x2592
//#define PIXEL_QUARTER           0x2591

short glbRenderMode = RM_GREYFILLED_PLUS;
float *pDepthBuffer = nullptr;

void InitDepthBuffer( int nScreenW, int nScreenH ) {
    // Depth buffer: every pixel on the screen has an associated floating point depth value
    pDepthBuffer = new float[nScreenW * nScreenH];
    ClearDepthBuffer( nScreenW, 0, 0, nScreenW, nScreenH );
}

void ClearDepthBuffer( int nScreenW, int x1, int y1, int x2, int y2 ) {
    for (int y = y1; y < y2; y++)
        for (int x = x1; x < x2; x++)
            pDepthBuffer[ y * nScreenW + x ] = 0.0f;
}

// A camera is defined by its location and orientation (in world space).
// Since the pitch, yaw and roll determine the orientation they are stored in the camera as well.
// The resulting projection and view matrices are part of the camera structure.
// For practical reasons also the viewport information is stored in the camera.

// Initializes the camera using the parameters. Calculates camera's projection matrix as well.

void camera::InitCamera( olc::PixelGameEngine *engine, std::string name, int x1, int y1, int x2, int y2,
                        float fieldOfViewInDegrees, float nearPlaneDistance, float farPlaneDistance ) {
    sCameraName = name;

    fCameraPitch = 0.0f;
    fCameraYaw   = 0.0f;
    fCameraRoll  = 0.0f;

    nViewPortX1 = x1;
    nViewPortY1 = y1;
    nViewPortX2 = x2;
    nViewPortY2 = y2;

    nViewPortWidth  = x2 - x1;
    nViewPortHeight = y2 - y1;

    fNearPlane = nearPlaneDistance;     // save these plane distances - they're needed for clipping
    fFarPlane  = farPlaneDistance;
    // Projection matrix is only determined once (at init time), because viewport dimensions and field of view
    // are not going to change in the application.
    // Input paramters are: field of view (in degrees), aspect ratio, near plane, far plane
    matProj = Matrix_MakeProjection( fieldOfViewInDegrees, (float)nViewPortHeight / (float)nViewPortWidth, nearPlaneDistance, farPlaneDistance );

    minRGBvalue =   0;
    maxRGBvalue = 255;

    gfxEngine = engine;
}

void camera::UpdateCamera( float fieldOfViewInDegrees, float nearPlaneDistance, float farPlaneDistance ) {

    fNearPlane = nearPlaneDistance;     // save these plane distances - they're needed for clipping
    fFarPlane  = farPlaneDistance;

    // Input paramters are: field of view (in degrees), aspect ratio, near plane, far plane
    matProj = Matrix_MakeProjection( fieldOfViewInDegrees, (float)nViewPortHeight / (float)nViewPortWidth, nearPlaneDistance, farPlaneDistance );
}

// ==============================/   Rendering code    /==============================

// Kind of clear screen for the viewport associated with the camera
void camera::ClearCameraViewPort( bool viewPortBorder ) {
    int x1 = nViewPortX1;
    int y1 = nViewPortY1;
    int x2 = nViewPortX2;
    int y2 = nViewPortY2;

    gfxEngine->FillRect( x1 - 1, y1 - 1, x2 - x1 + 1, y2 - y1 + 1, olc::BLACK );
    if (viewPortBorder) {
        for (int i = 0; i < 2; i++)
            gfxEngine->DrawRect( x1 - 1 - i, y1 - 1 - i, (x2 + i) - (x1 - 1 - i), (y2 + i) - (y1 - 1 - i), olc::YELLOW );
        if (sCameraName.size() > 0)
            gfxEngine->DrawString( x1 + 2, y1 + 2, sCameraName, olc::YELLOW );
    }
    // the depth buffer part corresponding to this viewport is also cleared
    ClearDepthBuffer( gfxEngine->ScreenWidth(), x1, y1, x2 + 1, y2 + 1 );
}

    // Whenever the fCameraPitch, -Yaw and/or -Roll are changed, this function can be called to recalculate both
    // the coordinate system of the camera and its view matrix
void camera::RecalculateCamera() {
    // Set the default look and up vectors along (resp.) the z and y axis as preparation on the rotation
    vec3d vUpTmp    = { 0, 1, 0 };
    vec3d vLookTmp  = { 0, 0, 1 };

    // build rotation matrix for camera using pitch, yaw and roll.
    // NOTE: Gimbal locks are minimized by choosing the right order to multiply the matrices - y is the parent axis, z is the grandchild axis
    mat4x4 matCameraRotX   = Matrix_MakeRotationX( fCameraPitch );
    mat4x4 matCameraRotY   = Matrix_MakeRotationY( fCameraYaw   );
    mat4x4 matCameraRotZ   = Matrix_MakeRotationZ( fCameraRoll  );

    mat4x4 matCameraRotZX  = Matrix_MultiplyMatrix( matCameraRotZ,  matCameraRotX );
    mat4x4 matCameraRotZXY = Matrix_MultiplyMatrix( matCameraRotZX, matCameraRotY );

    // Calculate camera coordinate axes vLookDir and vUp using the rotation matrix
    vLookDir  = Matrix_MultiplyVector( matCameraRotZXY, vLookTmp );
    vUp       = Matrix_MultiplyVector( matCameraRotZXY, vUpTmp );
    // Calculate vRight as a normal vector to vLookDir and vUp
    vRight    = Vector_CrossProduct( vLookDir, vUp );

    // Add look direction to camera location to provide a target for the camera to look at
    vec3d vTarget = Vector_Add( vPosition, vLookDir );
    // Create the PointAt matrix, providing camera location, target to look at and up vector. This PointAt matrix is an intermediate step
    // for calculating the view matrix (below).
    mat4x4 matCamera = Matrix_PointAt( vPosition, vTarget, vUp );
    // Apply the quick inverse on the PointAt matrix to calculate the View matrix (the PointAt matrix is no longer used hereafter).
    matView = Matrix_QuickInverse( matCamera );
}

void camera::Tri_PropagateColourInfo( triangle triIn, triangle &triOut ) {
    triOut.r = triIn.r;
    triOut.g = triIn.g;
    triOut.b = triIn.b;

    triOut.renderMode = triIn.renderMode;
    triOut.ptrSprite  = triIn.ptrSprite;
}

// Performs a (generic) transform from triIn to triOut, using transformation matrix trfMatrix.
// The texture coordinates as well as the col and sym values of the triangle are propagated from triIn to triOut.
void camera::Tri_Transform( triangle &triIn, mat4x4 trfMatrix, triangle &triOut ) {
    // transform each vertex of the triangle
    for (int i = 0; i < 3; i++) {
        triOut.p[i] = Matrix_MultiplyVector( trfMatrix, triIn.p[i] );
        triOut.t[i] = triIn.t[i];
    }
    // propagate colour info to transformed triangle
    Tri_PropagateColourInfo( triIn, triOut );
}

// Performs world transformation (from model to world space) using worldMatrix. As a result
// the input triIn is transformed into triOut
void camera::Tri_WorldTransform( triangle &triIn, mat4x4 &worldMatrix, triangle &triOut ) {
    Tri_Transform( triIn, worldMatrix, triOut );
}

// Performs view transformation (from world to view space) using viewMatrix. As a result
// the input triIn is transformed into triOut
void camera::Tri_ViewTransform( triangle &triIn, mat4x4 &viewMatrix, triangle &triOut ) {
    Tri_Transform( triIn, viewMatrix, triOut );
}

// Performs projection transformation (from view space to projection space) using projectionMatrix. As a result
// the input triIn is transformed into triOut
void camera::Tri_ProjectTransform( triangle &triIn, mat4x4 &projectionMatrix, triangle &triOut ) {
    Tri_Transform( triIn, projectionMatrix, triOut );
}

// Scales the coordinates of TriIn into the camera viewport.
void camera::Tri_ScaleIntoCameraView( triangle &triIn, triangle &triOut ) {
    // propagate colour value
    Tri_PropagateColourInfo( triIn, triOut );

    // propagate and normalize texture coordinates
    for (int i = 0; i < 3; i++) {
        triOut.t[i].u = triIn.t[i].u / triIn.p[i].w;
        triOut.t[i].v = triIn.t[i].v / triIn.p[i].w;
        triOut.t[i].w = 1.0f         / triIn.p[i].w;
    }

    // we moved the normalising into cartesian space
    // out of the matrix.vector function from the previous video, so do this manually
    triOut.p[0] = Vector_Div( triIn.p[0], triIn.p[0].w );
    triOut.p[1] = Vector_Div( triIn.p[1], triIn.p[1].w );
    triOut.p[2] = Vector_Div( triIn.p[2], triIn.p[2].w );

// REMARK - The following snippet was taken from the olc source code. In the video no mentioning nor
// explanation is given for this code. I still don't quite comprehend why this is necessary.
//
// NOTE: upto using the axis.obj file, this piece of code wasn't necessary. Can it be that the axis are flipped in
// their coordinate system? Or does the introduction of the camera flip the coordinates both in x and y direction?
//
// ADDITIONAL NOTE: in the original olc code both x and y are flipped:
//   * y-coordinate --> needs to be flipped because positive y in world coords is up, whereas in screen coords it is down
//   * x-coordinate --> must not be flipped. The reason Javidx9 did this was that the coordinates of the vertices in the
//                      object file were flipped to start with. [ the suspected cause behind this is exporting with a different
//                      orientation (right handed iso lefthanded or vice versa ]
//                      So if you start with an invalid base line, you must correct
//                      this in the code. However, the invalid vertex x-coordinates are only in axis.obj, so
//                      in general the x-coordinate must not be inverted!

    // X/Y are inverted so put them back
//    triOut.p[0].x *= -1.0f;
//    triOut.p[1].x *= -1.0f;
//    triOut.p[2].x *= -1.0f;
    triOut.p[0].y *= -1.0f;
    triOut.p[1].y *= -1.0f;
    triOut.p[2].y *= -1.0f;

// END_REMARK

    // coordinates in triOut are in -1, +1 range. First shift them to 0, +2 range
    vec3d vOffsetView = { 1.0f, 1.0f, 0.0f };
    triOut.p[0] = Vector_Add( triOut.p[0], vOffsetView );
    triOut.p[1] = Vector_Add( triOut.p[1], vOffsetView );
    triOut.p[2] = Vector_Add( triOut.p[2], vOffsetView );

    // now scale from 0, +2 range to 0, +1 range and then to viewport dimensions
    // also offset by viewport boundary
    triOut.p[0].x = triOut.p[0].x * 0.5f * (float)nViewPortWidth  + (float)nViewPortX1;
    triOut.p[0].y = triOut.p[0].y * 0.5f * (float)nViewPortHeight + (float)nViewPortY1;
    triOut.p[1].x = triOut.p[1].x * 0.5f * (float)nViewPortWidth  + (float)nViewPortX1;
    triOut.p[1].y = triOut.p[1].y * 0.5f * (float)nViewPortHeight + (float)nViewPortY1;
    triOut.p[2].x = triOut.p[2].x * 0.5f * (float)nViewPortWidth  + (float)nViewPortX1;
    triOut.p[2].y = triOut.p[2].y * 0.5f * (float)nViewPortHeight + (float)nViewPortY1;
}

// Performs culling, view transform and projection transform in the triangle inputTri. Because of clipping
// against the near plane, the result can be 0, 1 or 2 triangles, that are added to vecOfTris
void camera::CullViewAndProjectTriangle( triangle &inputTri, std::vector<triangle> &vecOfTris, vec3d vLightDir ) {

    triangle triTransformed, triViewed, triProjected, triFinal;
    std::vector<triangle> tmpVecOfTris;

    // The triangle passed as input parameters must not change - make a copy to prevent the original being overwritten
    triTransformed = inputTri;

    // This is where the normals of the triangles are considered to see if triangles are visible or not.
    vec3d normal, line1, line2;
    // Get lines either side of the triangle
    line1 = Vector_Sub( triTransformed.p[1], triTransformed.p[0] );   // determine line1 = line A
    line2 = Vector_Sub( triTransformed.p[2], triTransformed.p[0] );   // determine line2 = line B
    // Take cross product of lines to get normal vector to triangle surface
    normal = Vector_CrossProduct( line1, line2 );
    // Make sure the normal vector is normalized in length
    normal = Vector_Normalise( normal );

    // Backface culling is done by wrapping the projection code in an if block, depending on the normal

    // how much of the z-component of the triangles normal is aligned with the line
    // between the camera position and the location of the normal.

    // Get Ray from triangle to camera [ by subtracting 2 points you get a vector :))
    // p[0] is used here, but any point on the plane will do
    vec3d vCameraRay = Vector_Sub( triTransformed.p[0], vPosition );

    // culling if statement here!
    if (glbRenderMode == RM_WIREFRAME || glbRenderMode == RM_WIREFRAME_RGB || Vector_DotProduct( normal, vCameraRay ) < 0.0f) {

        // Add illumination, to make the 3d objects more intuitive
        // Single direction lighting - light is shining towards the player
//        vec3d light_direction = { 0.0f, 0.0f, -1.0f };
        vec3d light_direction = Vector_Normalise( vLightDir );

        // determine the alignment between the normal and the light direction
        float dot_prod = std::max( 0.0f, Vector_DotProduct( light_direction, normal ));
        // use the alignment to determine the grey shade, and store it in the triangle
        GetColour2( dot_prod, triTransformed );     // alternatively use GetColour()

        // Before clipping and projection, first transform from world space to view space
        Tri_ViewTransform( triTransformed, matView, triViewed );

        // ====================/  Clipping against near plane  /====================

        // Clip Viewed Triangle against near plane, this could form two additional triangles.
        // the array is for retrieving the resulting triangles
        int nClippedTriangles = 0;
        triangle clipped[2];

        // the first parameter is a point on the near plane, the second is the normal to the near plane
        nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, fNearPlane }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

        // We may end up with multiple triangles form the clip, store them for clipping against the far plane
        for (int n = 0; n < nClippedTriangles; n++)
            tmpVecOfTris.push_back( clipped[n] );

        // ====================/  Clipping against far plane  /====================

        for (auto tri : tmpVecOfTris) {
            // Clip Viewed Triangle against far plane, this could form two additional triangles per triangle.
            nClippedTriangles = 0;

            // the first parameter is a point on the far plane, the second is the normal to the far plane
            nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, fFarPlane }, { 0.0f, 0.0f, -1.0f }, tri, clipped[0], clipped[1]);

            // We may end up with multiple triangles form the clip, so project 0, 1 or 2 as required
            for (int n = 0; n < nClippedTriangles; n++) {

                // Project the points of the triangle from 3D -->  2D
                Tri_ProjectTransform( clipped[n], matProj, triProjected );

                // scale into view - i.e. normalize, invert x and y, and scale to viewport dimensions
                Tri_ScaleIntoCameraView( triProjected, triFinal );

                // Store the triangles that are going to be drawn for sorting
                vecOfTris.push_back( triFinal );
            }
        }
    }
}

// Performs the rasterizing of all the triangles in the vector trisToRaster
void camera::RasterizeTriangles( std::vector<triangle> &trisToRaster, std::vector<triangle> &trisToRender ) {

//    if (glbRenderMode == RM_GREYFILLED || glbRenderMode == RM_GREYFILLED_PLUS || glbRenderMode == RM_WIREFRAME || glbRenderMode == RM_WIREFRAME_RGB) {
        // Sort triangles from back to front - using a function from the algorithm standard lib
        // standard function sort() requires starting point, ending point, and sorting criterium
        // This implements the painting algorithm for drawing.
        sort( trisToRaster.begin(), trisToRaster.end(),
             // this lambda provides the sorting criterium
             [](triangle &t1, triangle &t2) {
                 // determine z-value of midpoints for both triangles
                 float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
                 float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
                 // return if they are in the right ordering already (the sorting criterium)
                 return z1 > z2;
             });
//    }

    std::list<triangle> listTriangles;    // this is the queue from the slides / notes (a std::list)

    for (auto &rasterTri : trisToRaster) {
        // Clip triangles against all four screen edges, this could yield a bunch of triangles, so create a queue
        // that we traverse to ensure we only test new triangles generated against planes
        triangle clipped[2];

        // Add initial triangle
        listTriangles.clear();
        listTriangles.push_back(rasterTri);
        int nNewTriangles = 1;

        // look at the pseudo code in the notes for explanation
        for (int p = 0; p < 4; p++) {   // iterate 4 planes
            int nTrisToAdd = 0;
            while (nNewTriangles > 0) {
                // Take triangle from front of queue
                triangle test = listTriangles.front();
                listTriangles.pop_front();
                nNewTriangles--;

                // Clip it against a plane. We only need to test each subsequent plane, against subsequent new triangles
                // as all triangles after a plane clip are guaranteed to lie on the inside of the plane.
                vec3d clipPIP, clipNormal;   // point in plane to clip, normal vector on plane to clip
                switch (p) {
                    case 0:	clipPIP = { 0.0f, (float)nViewPortY1 + 0.1f, 0.0f }; clipNormal = {  0.0f,  1.0f, 0.0f }; break;  // top        of viewport, normal pointing downwards
                    case 1:	clipPIP = { 0.0f, (float)nViewPortY2 - 1.0f, 0.0f }; clipNormal = {  0.0f, -1.0f, 0.0f }; break;  // bottom     of viewport, normal pointing upwards
                    case 2:	clipPIP = { (float)nViewPortX1 + 0.1f, 0.0f, 0.0f }; clipNormal = {  1.0f,  0.0f, 0.0f }; break;  // left side  of viewport, normal pointing right
                    case 3:	clipPIP = { (float)nViewPortX2 - 1.0f, 0.0f, 0.0f }; clipNormal = { -1.0f,  0.0f, 0.0f }; break;  // right side of viewport, normal pointing left
                }
                nTrisToAdd = Triangle_ClipAgainstPlane( clipPIP, clipNormal, test, clipped[0], clipped[1]);

                // Clipping may yield a variable number of triangles, so add these new ones to the back of the queue
                // for subsequent clipping against next planes
                for (int w = 0; w < nTrisToAdd; w++)
                    listTriangles.push_back(clipped[w]);
            }
            nNewTriangles = listTriangles.size();
        }
        // store all processed triangles in a separate vector for rendering later on.
        for (auto &t : listTriangles)
            trisToRender.push_back( t );
    }
}

/* GetColour stuff:
 * In the pixelGameEngine, the primary colours are defined in constants for Red, Green, Blue in intensity ranging 0 - 255.
 *     for instance: GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
 * In between them variants are possible by varying the second parameter of the pixel-triplet.
 * The following two functions use this feature.
 */

// In the GetColour() function a minimum and maximum RGB-value is used. Theoretically these values are 0 - 255.
// To get a better visibility, you can set other minimum and maximum RGB values for the
// grey colouring.
// If minVal and maxVal don't comply with 0 <= minVal < maxVal <= 255, then nothing is changed.
void camera::SetRGBrange( int minVal, int maxVal ) {
    if ( 0 <= minVal && minVal < maxVal && maxVal <= 255) {
        minRGBvalue = (short)minVal;
        maxRGBvalue = (short)maxVal;
    }
}

// This function translates a variable a with value in a range between a_min and a_max into
// a corresponding (i.e. proportional) value in the range b_min to b_max.
int camera::VaryShade( float a, float a_min, float a_max, int b_min, int b_max ) {
     return (int)(((a - a_min) / (a_max - a_min)) * (float)(b_max - b_min)) + b_min;
}

// This function translates a variable a with value in a range between a_min and a_max into
// a corresponding (i.e. proportional) result in the range b_max to b_min.
// Thus if a == a_min, then result == b_max !!
int camera::VaryShadeReversed( float a, float a_min, float a_max, int b_min, int b_max ) {
    return b_max - VaryShade( a, a_min, a_max, b_min, b_max );
}

// This function is a variant of GetColour(). It uses less black and white shades and thus makes
// the shades more distinguishable. I noticed that so many black or near black shades were rendered
// that I often felt like the regular GetColour() wasn't working at all :)
void camera::GetColour2( float lum, triangle &tri ) {

// This part is for the pixelGameEngine
    int grey_rgb = VaryShade( lum, 0.0f, 1.0f, minRGBvalue, maxRGBvalue );
    tri.r = grey_rgb;
    tri.g = grey_rgb;
    tri.b = grey_rgb;
}

// Clipping function, returns the number of triangles that are created by it.
// inputs:   plane_p, plane_n --> the plane equation parameters (a point in the plane and the normal vector to the plane)
//           in_tri           --> the triangle to be clipped
// outputs:  out_tri1 and out_tri2 (either neither, or the first, or both triangles will be useful)
int camera::Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2) {
    int returnVal = -1;

    // Make sure plane normal is indeed normal
    plane_n = Vector_Normalise(plane_n);

    // Create two temporary storage arrays to classify points either side of plane
    // If distance sign is positive, point lies on "inside" of plane
    vec3d*  inside_points[3]; int  nInsidePointCount = 0;
    vec3d* outside_points[3]; int nOutsidePointCount = 0;
    vec2d*  inside_tex[   3]; int  nInsideTexCount   = 0;
    vec2d* outside_tex[   3]; int nOutsideTexCount   = 0;

    // Get signed shortest distance of each point in triangle to plane
    float d0 = Vector_Distance( plane_n, plane_p, in_tri.p[0] );
    float d1 = Vector_Distance( plane_n, plane_p, in_tri.p[1] );
    float d2 = Vector_Distance( plane_n, plane_p, in_tri.p[2] );

    if (d0 >= 0) {  inside_points[ nInsidePointCount++] = &in_tri.p[0];  inside_tex[ nInsideTexCount++] = &in_tri.t[0]; }
    else {         outside_points[nOutsidePointCount++] = &in_tri.p[0]; outside_tex[nOutsideTexCount++] = &in_tri.t[0]; }
    if (d1 >= 0) {  inside_points[ nInsidePointCount++] = &in_tri.p[1];  inside_tex[ nInsideTexCount++] = &in_tri.t[1]; }
    else {         outside_points[nOutsidePointCount++] = &in_tri.p[1]; outside_tex[nOutsideTexCount++] = &in_tri.t[1]; }
    if (d2 >= 0) {  inside_points[ nInsidePointCount++] = &in_tri.p[2];  inside_tex[ nInsideTexCount++] = &in_tri.t[2]; }
    else {         outside_points[nOutsidePointCount++] = &in_tri.p[2]; outside_tex[nOutsideTexCount++] = &in_tri.t[2]; }

    // Now classify triangle points, and break the input triangle into
    // smaller output triangles if required. There are four possible
    // outcomes...

    if (nInsidePointCount == 0) {
        // All points lie on the outside of plane, so clip whole triangle - It ceases to exist
        returnVal = 0; // No returned triangles are valid
    } else  if (nInsidePointCount == 3) {
        // All points lie on the inside of plane, so do nothing and allow the triangle to simply pass through
        out_tri1 = in_tri;
        returnVal = 1; // Just the one returned original triangle is valid
    } else if (nInsidePointCount == 1 && nOutsidePointCount == 2) {
        // Triangle should be clipped. As two points lie outside the plane, the triangle simply becomes a smaller triangle

        // Copy appearance info to new triangle
        Tri_PropagateColourInfo( in_tri, out_tri1 );

        // The inside point is valid, so keep that...
        out_tri1.p[0] = *inside_points[0];
        out_tri1.t[0] = *inside_tex[0];

        // but the two new points are at the locations where the original sides of the triangle (lines) intersect with the plane
        float t;
        out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
        out_tri1.t[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
        out_tri1.t[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
        out_tri1.t[1].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
        out_tri1.t[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
        out_tri1.t[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;
        out_tri1.t[2].w = t * (outside_tex[1]->w - inside_tex[0]->w) + inside_tex[0]->w;

        returnVal = 1; // Return the newly formed single triangle
    } else if (nInsidePointCount == 2 && nOutsidePointCount == 1) {
        // Triangle should be clipped. As two points lie inside the plane, the clipped triangle becomes a "quad". Fortunately, we can
        // represent a quad with two new triangles

        // Copy appearance info to new triangles
        Tri_PropagateColourInfo( in_tri, out_tri1 );
        Tri_PropagateColourInfo( in_tri, out_tri2 );

        // The first triangle consists of the two inside points and a new point determined by the location where one side of the triangle
        // intersects with the plane
        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = *inside_points[1];
        out_tri1.t[0] = *inside_tex[0];
        out_tri1.t[1] = *inside_tex[1];

        float t;
        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
        out_tri1.t[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
        out_tri1.t[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;
        out_tri1.t[2].w = t * (outside_tex[0]->w - inside_tex[0]->w) + inside_tex[0]->w;

        // The second triangle is composed of one of the inside points, a
        // new point determined by the intersection of the other side of the
        // triangle and the plane, and the newly created point above
        out_tri2.p[0] = *inside_points[1];
        out_tri2.t[0] = *inside_tex[1];

// the original code did not preserve the strict clockwise ordering, hence a correction. JvdB 19-06-2021

//        out_tri2.p[1] = out_tri1.p[2];
//        out_tri2.t[1] = out_tri1.t[2];
//        out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
//        out_tri2.t[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
//        out_tri2.t[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
//        out_tri2.t[2].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;

// Correction after discussion on discord on this topic. JvdB 19-06-2021

        out_tri2.p[2] = out_tri1.p[2];
        out_tri2.t[2] = out_tri1.t[2];
        out_tri2.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
        out_tri2.t[1].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
        out_tri2.t[1].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
        out_tri2.t[1].w = t * (outside_tex[0]->w - inside_tex[1]->w) + inside_tex[1]->w;

        returnVal = 2; // Return two newly formed triangles which form a quad
    } else
        std::cout << "ERROR: Triangle_ClipAgainstPlane() --> unexpected combination of inside/outside points"  << std::endl;

    return returnVal;
}

// This function calculates the intersection line segment between a triangle (in_tri) and a plane, described by
// its normal and a point in the plane. It returns 0 if no intersection was found, 1 otherwise. Returns -1 upon error.
// inputs:   plane_p, plane_n --> the plane equation parameters (a point in the plane and the normal vector to the plane)
//           in_tri           --> the triangle to be checked against the plane
// outputs:  out_point1 and out_point2 (either neither, or both points will be useful)
int Triangle_IntersectPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, vec3d &out_point1, vec3d &out_point2 ) {
    int returnVal = -1;

    // Make sure plane normal is indeed normal
    plane_n = Vector_Normalise(plane_n);

    // Create two temporary storage arrays to classify points either side of plane
    // If distance sign is positive, point lies on "inside" of plane
    vec3d*  inside_points[3]; int  nInsidePointCount = 0;
    vec3d* outside_points[3]; int nOutsidePointCount = 0;
    vec2d*  inside_tex[   3]; int  nInsideTexCount   = 0;
    vec2d* outside_tex[   3]; int nOutsideTexCount   = 0;

    // Get signed shortest distance of each point in triangle to plane
    float d0 = Vector_Distance( plane_n, plane_p, in_tri.p[0] );
    float d1 = Vector_Distance( plane_n, plane_p, in_tri.p[1] );
    float d2 = Vector_Distance( plane_n, plane_p, in_tri.p[2] );

    if (d0 >= 0) {  inside_points[ nInsidePointCount++] = &in_tri.p[0];  inside_tex[ nInsideTexCount++] = &in_tri.t[0]; }
    else {         outside_points[nOutsidePointCount++] = &in_tri.p[0]; outside_tex[nOutsideTexCount++] = &in_tri.t[0]; }
    if (d1 >= 0) {  inside_points[ nInsidePointCount++] = &in_tri.p[1];  inside_tex[ nInsideTexCount++] = &in_tri.t[1]; }
    else {         outside_points[nOutsidePointCount++] = &in_tri.p[1]; outside_tex[nOutsideTexCount++] = &in_tri.t[1]; }
    if (d2 >= 0) {  inside_points[ nInsidePointCount++] = &in_tri.p[2];  inside_tex[ nInsideTexCount++] = &in_tri.t[2]; }
    else {         outside_points[nOutsidePointCount++] = &in_tri.p[2]; outside_tex[nOutsideTexCount++] = &in_tri.t[2]; }

    // Now classify triangle points, and break the input triangle into smaller output triangles if required.
    // There are four possible outcomes...

    if (nInsidePointCount == 0 || nInsidePointCount == 3) {
        // All points lie on the outside or on the inside of the plane, so there's no intersection line segment.
        returnVal = 0;
    } else if (nInsidePointCount == 1 && nOutsidePointCount == 2) {

        // the two points that define the intersection line segment are at the locations where the original sides
        // of the triangle (lines) intersect with the plane
        out_point1 = Vector_IntersectPlane( plane_p, plane_n, *inside_points[0], *outside_points[0] );
        out_point2 = Vector_IntersectPlane( plane_p, plane_n, *inside_points[0], *outside_points[1] );

        returnVal = 1;

    } else if (nInsidePointCount == 2 && nOutsidePointCount == 1) {

        // The first triangle consists of the two inside points and a new point determined by the location where one side of the triangle
        // intersects with the plane
        out_point1 = Vector_IntersectPlane( plane_p, plane_n, *inside_points[0], *outside_points[0] );
        out_point2 = Vector_IntersectPlane( plane_p, plane_n, *inside_points[1], *outside_points[0] );

        returnVal = 1;
    } else
        std::cout << "ERROR: Triangle_IntersectPlane() --> unexpected combination of inside/outside points"  << std::endl;

    return returnVal;
}

