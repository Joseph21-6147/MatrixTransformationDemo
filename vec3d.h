#ifndef VEC3D_H
#define VEC3D_H

#include <fstream>

// DATATYPES

struct vec2d {          // used for textures, that are a 2d thing
    float u = 0.0f;     // the name convention is common in texturing
    float v = 0.0f;
    float w = 1.0f;     // Need a 3d term to perform sensible sprite operations
};

struct vec3d {          // (vector to) a point in 3d space
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;     // Need a 4th term to perform sensible matrix vector multiplication
};

// PROTOTYPES GENERIC VECTOR FUNCTIONS

vec3d Vector_Add( vec3d &v1, vec3d &v2 );           // adds vector v1 and v2, and returns resulting vector
vec3d Vector_Sub( vec3d &v1, vec3d &v2 );           // subtracts vector v1 and v2, and returns resulting vector
vec3d Vector_Mul( vec3d &v , float k   );           // multiplies vector v with scalar k, and returns resulting vector
vec3d Vector_Div( vec3d &v , float k   );           // divides (float) vector v with scalar k, and returns resulting vector

float Vector_Length(       vec3d &v );              // calculates vector length using pythagoras in 3D
vec3d Vector_Normalise(    vec3d &v );              // divides each vector member by the length of the vector
float Vector_DotProduct(   vec3d &v1, vec3d &v2 );  // performs dot product of 2 vectors and returns result
vec3d Vector_CrossProduct( vec3d &v1, vec3d &v2 );  // performs cross product of 2 vectors and returns result
                                                    // the cross product can be used to create a normal vector on a plane defined
                                                    // by the two vectors v1 and v2
// PROTOTYPES SPECIAL VECTOR FUNCTIONS

// a, b, c are vertices of a triangle, in clockwise order. This function returns the
// unit normal vector on the triangle.
vec3d Vector_GetNormal( vec3d &a, vec3d &b, vec3d &c );
// plane_n is a unit normal vector on a plane, and plane_p is a point in that plane.
// p is a vertex in 3D. The function returns the (signed) shortest distance from p to the plane.
float Vector_Distance( vec3d plane_n, vec3d plane_p, vec3d p );

// Tests and return the point where a line intersects with a plane.
// The plane is defined by a point on the plane (plane_p) and a normal vector to the plane (plane_n).
// The line to check on intersection is defined by points (in 3D) lineStart and lineEnd
// The function returns the point where the line intersects with the plane. The algorithm can be found on internet

// The variant with the additional parameter t is used in textured graphics. The function is identical but the
// value of t can be used to interpolate the texture.
vec3d Vector_IntersectPlane( vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd );
vec3d Vector_IntersectPlane( vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd, float &t );

// UTILITY FUNCTIONS

 void  Vector_Print( vec3d &v, bool end_line );          // outputs to cout the 4 elements of the vector. Outputs endl if boolean is true
 vec3d Vector_Get( float x, float y, float z, float w ); // creates and returns a vector with the four values given as parameters

// use for vec2d vectors - prints the contents of a vec2d to a string and returns it
std::string Vector_PrintToString2( std::string header, vec2d &v );

// use for vec3d vectors - prints the contents of a vec3d to a string and returns it
std::string Vector_PrintToString3( std::string header, vec3d &v );

#endif // VEC3D_H
