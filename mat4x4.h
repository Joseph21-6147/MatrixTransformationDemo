#ifndef MAT4X4_H
#define MAT4X4_H

#include  "vec3d.h"

// CONSTANTS

#define PI 3.1415926535f

// DATATYPES

struct mat4x4 {
    // matrix compatible with vec3d - ordering is row - column
    float m[4][4] = { 0 };
};

// FUNCTION PROTOTYPES

// Returns the matrix multiplication result between vector i and matrix m.
// The vector is considered a row vector, so the i-th element of the vector is
// calculated using column i of the matrix.
vec3d Matrix_MultiplyVector( mat4x4 &m, vec3d &v );

// Builds a matrix using the 16 parameters. Both rows and columns count from 0 to 3.
// Returns the resulting matrix.
mat4x4 Matrix_Buildup( float r0c0, float r0c1, float r0c2, float r0c3,
                       float r1c0, float r1c1, float r1c2, float r1c3,
                       float r2c0, float r2c1, float r2c2, float r2c3,
                       float r3c0, float r3c1, float r3c2, float r3c3 );

// Creates and returns an identity matrix.
mat4x4 Matrix_MakeIdentity();

// Note: sometimes you encounter rotation matrix versions that are transposed wrt these below.
// This depends on the choice if you look at the vertices as row vectors and post-multiply the rotation matrix.
// If you use column vector representation you have to pre multiply the rotation matrix.
// These functions consider points to be represented as row vectors !!

// Returns the matrix for rotation around the X-axis with angle theta.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeRotationX( float theta );

// Returns the matrix for rotation around the Y-axis with angle theta.
// IMPORTANT NOTE: this transformation matrix is to be used with points represented as row-vectors!
mat4x4 Matrix_MakeRotationY( float theta );

// Returns the matrix for rotation around the Z-axis with angle theta.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeRotationZ( float theta );

// Returns the translation matrix with offsets in Tx, Ty and Tz.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeTranslation( float Tx, float Ty, float Tz );

// Returns the scaling matrix with scaling factors in Sx, Sy and Sz.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeScaling( float Sx, float Sy, float Sz );

// Returns a projection matrix based on the four input parameters.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeProjection( float fFovDegrees, float fAspectRatio, float fNear, float fFar );

// Returns the result of matrix multiplication of m1 and m2.
// the new value at (r, c) is constructed using the row vector r of m1 and the column vector c of m2
mat4x4 Matrix_MultiplyMatrix(  mat4x4 &m1, mat4x4 &m2 );

// Creates and returns a complete transformation matrix using the scale factors, the rotation angles
// and translation distances.
mat4x4 Matrix_MakeTransformComplete( float xScale, float yScale, float zScale,
                                     float xAngle, float yAngle, float zAngle,
                                     float xTrnsl, float yTrnsl, float zTrnsl );

// Intuitively the point-at matrix is used to make translations from the world coordinate system to the camera coordinate system.
// It doesn't translate anything into the world itself, but only puts the camera in the correct world coordinates.
// pos is the position of the camera, target is the "forward vector", up is the up vector
mat4x4 Matrix_PointAt( vec3d &pos, vec3d &target, vec3d &up );

// Matrix_QuickInverse() creates the "Look-At" - matrix. This is the matrix that translates the world coordinates into camera (view)
// coordinates. The Look-At matrix is also known as view matrix
// IMPORTANT NOTE: only for Rotation/Translation Matrices - DOES NOT WORK in combination with scaling matrices
mat4x4 Matrix_QuickInverse( mat4x4 &m );

// Prints the contents of a matrix to a string, and returns the string
// can be used to std::cout printing or for saving to a file
std::string Matrix_PrintToString( std::string header, mat4x4 &m );

#endif // MAT4X4_H


