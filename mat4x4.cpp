#include "mat4x4.h"
#include <cmath>

// ===== matrix utility functions - implementation ----- //

// Returns the matrix multiplication result of vector i and matrix m.
// The vector is considered a row vector, so the i-th element of the vector is
// calculated using column i of the matrix.
vec3d Matrix_MultiplyVector( mat4x4 &m, vec3d &i ) {
    vec3d v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
    v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
    return v;
}

// Builds a matrix using the 16 parameters. Both rows and columns count from 0 to 3.
// Returns the resulting matrix.
mat4x4 Matrix_Buildup( float r0c0, float r0c1, float r0c2, float r0c3,
                       float r1c0, float r1c1, float r1c2, float r1c3,
                       float r2c0, float r2c1, float r2c2, float r2c3,
                       float r3c0, float r3c1, float r3c2, float r3c3 ) {
    mat4x4 matrix;
    matrix.m[0][0] = r0c0;  matrix.m[0][1] = r0c1;  matrix.m[0][2] = r0c2;  matrix.m[0][3] = r0c3;
    matrix.m[1][0] = r1c0;  matrix.m[1][1] = r1c1;  matrix.m[1][2] = r1c2;  matrix.m[1][3] = r1c3;
    matrix.m[2][0] = r2c0;  matrix.m[2][1] = r2c1;  matrix.m[2][2] = r2c2;  matrix.m[2][3] = r2c3;
    matrix.m[3][0] = r3c0;  matrix.m[3][1] = r3c1;  matrix.m[3][2] = r3c2;  matrix.m[3][3] = r3c3;
    return matrix;
}

// Creates and returns an identity matrix.
mat4x4 Matrix_MakeIdentity() {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

// Note: sometimes you encounter rotation matrix versions that are transposed wrt these below.
// This depends on the choice if you look at the vertices as row vectors and post-multiply the rotation matrix.
// If you use column vector representation you have to pre multiply the rotation matrix.
// These functions consider points as row vectors !!

// Returns the matrix for rotation around the X-axis with angle theta.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeRotationX( float fAngleRad ) {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] =  cosf( fAngleRad );
    matrix.m[1][2] =  sinf( fAngleRad );
    matrix.m[2][1] = -sinf( fAngleRad );
    matrix.m[2][2] =  cosf( fAngleRad );
    matrix.m[3][3] = 1.0f;
    return matrix;
}

// Returns the matrix for rotation around the Y-axis with angle theta.
// IMPORTANT NOTE: this transformation matrix is to be used with points represented as row-vectors!
mat4x4 Matrix_MakeRotationY( float fAngleRad ) {
    mat4x4 matrix;
    matrix.m[0][0] =  cosf( fAngleRad );
    matrix.m[0][2] =  sinf( fAngleRad );
    matrix.m[2][0] = -sinf( fAngleRad );
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] =  cosf( fAngleRad );
    matrix.m[3][3] = 1.0f;
    return matrix;
}

// Returns the matrix for rotation around the Z-axis with angle theta.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeRotationZ( float fAngleRad ) {
    mat4x4 matrix;
    matrix.m[0][0] =  cosf( fAngleRad );
    matrix.m[0][1] =  sinf( fAngleRad );
    matrix.m[1][0] = -sinf( fAngleRad );
    matrix.m[1][1] =  cosf( fAngleRad );
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
}

// Returns the translation matrix with offsets in Tx, Ty and Tz.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeTranslation( float x, float y, float z ) {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
}

// Returns the scaling matrix with scaling factors in Sx, Sy and Sz.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeScaling( float Sx, float Sy, float Sz ) {
    mat4x4 matrix = Matrix_Buildup(   Sx, 0.0f, 0.0f, 0.0f,
                                    0.0f,   Sy, 0.0f, 0.0f,
                                    0.0f, 0.0f,   Sz, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f );
    return matrix;
}

// Returns a projection matrix based on the four input parameters.
// IMPORTANT NOTE: this transformation matrix is only to be used with points represented as row-vectors!
mat4x4 Matrix_MakeProjection( float fFovDegrees, float fAspectRatio, float fNear, float fFar ) {

    float fFovRadInverted = 1.0f / tanf(0.5f * fFovDegrees * PI / 180.0f);
    mat4x4 matrix;

    // now populate the matrix: first index denotes row, second index denotes column of the matrix
    matrix.m[0][0] = fAspectRatio * fFovRadInverted;
    matrix.m[1][1] = fFovRadInverted;

// some smart guy pointed out that there was a smarter way that was mathematically identical
//    matrix.m[2][2] =   fFar          / (fFar - fNear);
//    matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix.m[2][2] =   1.0f          / (fFar - fNear);
    matrix.m[3][2] = (-1.0f * fNear) / (fFar - fNear);

    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;

    return matrix;
}

// Returns the result of matrix multiplication of m1 and m2.
// the new value at (r, c) is constructed using the row vector r of m1 and the column vector c of m2
mat4x4 Matrix_MultiplyMatrix( mat4x4 &m1, mat4x4 &m2 ) {
    mat4x4 matrix;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] +
                             m1.m[r][1] * m2.m[1][c] +
                             m1.m[r][2] * m2.m[2][c] +
                             m1.m[r][3] * m2.m[3][c];
    return matrix;
}

// Creates and returns a transformation matrix using the scale factors, the rotation angles and
// translation distances in the parameter list.
mat4x4 Matrix_MakeTransformComplete( float xScale, float yScale, float zScale,
                                     float xAngle, float yAngle, float zAngle,
                                     float xTrnsl, float yTrnsl, float zTrnsl ) {
        // Set up rotation matrices, then translation matrix, and then build transform matrix
        // by multiplying all matrices in the right order
        mat4x4 matScale = Matrix_MakeScaling( xScale, yScale, zScale );
        mat4x4 matRotX  = Matrix_MakeRotationX( xAngle );
        mat4x4 matRotY  = Matrix_MakeRotationY( yAngle );
        mat4x4 matRotZ  = Matrix_MakeRotationZ( zAngle );
        mat4x4 matTrans = Matrix_MakeTranslation( xTrnsl, yTrnsl, zTrnsl );

        mat4x4 matrix;
        matrix   = Matrix_MultiplyMatrix( matRotY, matScale );
        matrix   = Matrix_MultiplyMatrix( matrix,  matRotZ  );
        matrix   = Matrix_MultiplyMatrix( matrix,  matRotX  );
        matrix   = Matrix_MultiplyMatrix( matrix,  matTrans );

        return matrix;
}

// Intuitively the point-at matrix is used to make translations from the world coordinate system to the camera coordinate system.
// It doesn't translate anything into the world itself, but only puts the camera in the correct world coordinates.
//
// pos is the position of the object to point at, target is the "forward vector", up is the up vector
mat4x4 Matrix_PointAt( vec3d &pos, vec3d &target, vec3d &up ) {
    // Calculate the new forward direction
    vec3d newForward = Vector_Sub( target, pos );
    newForward = Vector_Normalise( newForward );

    // Calculate new up direction, by calculating how much of the described up vector
    // projects onto the new forward vector.
    vec3d a = Vector_Mul( newForward, Vector_DotProduct( up, newForward ));
    vec3d newUp = Vector_Sub( up, a );
    newUp = Vector_Normalise( newUp );

    // New right direction is easy, its just cross product
    vec3d newRight = Vector_CrossProduct( newUp, newForward );

    // Construct dimensioning and translation matrix
    mat4x4 matrix;
    matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
    return matrix;
}

// Matrix_QuickInverse() creates the "LookAt" - matrix. This is the matrix that translates the world coordinates into camera (view) coordinates.
mat4x4 Matrix_QuickInverse(mat4x4 &m) { // Only for Rotation/Translation Matrices
    mat4x4 matrix;
    matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;

    matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
    matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
    matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
    matrix.m[3][3] = 1.0f;
    return matrix;
}

// Prints the contents of a matrix to a string, and returns the string
// can be used to cout printing or for saving to a file
std::string Matrix_PrintToString( std::string header, mat4x4 &m ) {

    std::string s;
    s.append( header );
    s.append( "\n");

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            s.append( " (row " + std::to_string( r ));
            s.append(  " col " + std::to_string( c ));
            s.append( " ) ");
            s.append( std::to_string( m.m[r][c] ));
        }
        s.append( "\n" );
    }
    return s;
}

