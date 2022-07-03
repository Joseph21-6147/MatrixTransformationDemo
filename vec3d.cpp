#include "vec3d.h" // contains data types and prototypes

#include <iostream>
#include <cmath>
#include <conio.h>

// IMPLEMENTATION

// adds vectors v1 and v2, and returns the resulting vector
vec3d Vector_Add( vec3d &v1, vec3d &v2 ) {
    return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

// subtracts vector v2 from vector v1, and returns the resulting vector
vec3d Vector_Sub( vec3d &v1, vec3d &v2 ) {
    return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

// multiplies vector v1 with scalar k, and returns the resulting vector
vec3d Vector_Mul( vec3d &v1, float k ) {
    return { v1.x * k, v1.y * k, v1.z * k };
}

// multiplies vector v1 by vector v2, and returns the resulting vector
// NOTE - this identical to dot product of two vectors
vec3d Vector_Mul( vec3d &v1, vec3d &v2 ) {
    return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
}

// divides vector v1 by scalar k, and returns the resulting vector
vec3d Vector_Div( vec3d &v1, float k ) {
    return { v1.x / k, v1.y / k, v1.z / k };
}

// divides vector v1 by vector v2, and returns the resulting vector
vec3d Vector_Div( vec3d &v1, vec3d &v2 ) {
    return { v1.x / v2.x, v1.y / v2.y, v1.z / v2.z };
}

// Function Dot product (Xa * Xb + Ya * Yb + Za * Zb) - gives a number of the "similarity" of two vectors.
//  1 --> if the are completely aligned
//  0 --> if one is perpendicular to the other
// -1 --> if one is opposite to the other
float Vector_DotProduct( vec3d &v1, vec3d &v2 ) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// returns the length of vector v
float Vector_Length( vec3d &v ) {
    return sqrtf( Vector_DotProduct( v, v ));
}

// normalises v and returns the normalised vector as result
vec3d Vector_Normalise( vec3d &v ) {
    return Vector_Div( v, Vector_Length( v ));
}

// the cross product can be used to create a normal vector on a plane defined
// by the two vectors v1 and v2
vec3d Vector_CrossProduct( vec3d &v1, vec3d &v2 ) {
    vec3d v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

// a, b, c are vertices of a triangle, in clockwise order. This function returns the
// unit normal vector on the triangle.
vec3d Vector_GetNormal( vec3d &a, vec3d &b, vec3d &c ) {
    vec3d line1  = Vector_Sub( b, a );
    vec3d line2  = Vector_Sub( c, a );
    vec3d normal = Vector_CrossProduct( line1, line2 );
    vec3d result = Vector_Normalise( normal );
    return result;
}

// plane_n is a unit normal vector on a plane, and plane_p is a point in that plane.
// p is a vertex in 3D. The function returns the (signed) shortest distance from p to the plane.
float Vector_Distance( vec3d plane_n, vec3d plane_p, vec3d p ) {
    vec3d v = Vector_Sub( p, plane_p );
    return Vector_DotProduct( v, plane_n );
}

// test and return the point where a line intersects with a plane.
// plane_p is a point on the plane, plane_n is the normal to the plane (together they define the plane equation)
// the line is specified by its starting (lineStart) and ending point (lineEnd)
vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd) {

    // Javidx9: "this is a standard (easy to find online) algorithm, nothing clever"
    // Joost: see for an explanation: https://math.stackexchange.com/questions/2041296/algorithm-for-line-in-plane-intersection-in-3d
    plane_n       = Vector_Normalise(plane_n);
    float plane_d = -Vector_DotProduct(plane_n, plane_p);
    float ad      = Vector_DotProduct(lineStart, plane_n);
    float bd      = Vector_DotProduct(lineEnd, plane_n);
    float t       = (-plane_d - ad) / (bd - ad);
    vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
    vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
    return Vector_Add(lineStart, lineToIntersect);
}

// identical version with the t parameter passed back
vec3d Vector_IntersectPlane( vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd, float &t ) {
    plane_n       =   Vector_Normalise(  plane_n );    // make sure the normal vector is normalised
    float plane_d = - Vector_DotProduct( plane_n,   plane_p );
    float ad      =   Vector_DotProduct( lineStart, plane_n );
    float bd      =   Vector_DotProduct( lineEnd,   plane_n );
    t             = (- plane_d - ad) / (bd - ad);
    vec3d lineStartToEnd  = Vector_Sub( lineEnd, lineStart );
    vec3d lineToIntersect = Vector_Mul( lineStartToEnd, t );
    return Vector_Add( lineStart, lineToIntersect );
}

// experimental alternative
vec3d Vector_IntersectPlane2(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd) {

    // see for an explanation: https://math.stackexchange.com/questions/2041296/algorithm-for-line-in-plane-intersection-in-3d
    plane_n = Vector_Normalise(plane_n);
    float plane_d = -Vector_DotProduct(plane_n, plane_p);

    vec3d v = Vector_Sub( lineEnd, lineStart );
    float t = (-plane_d - Vector_DotProduct(lineStart, plane_n)) / Vector_DotProduct(v, plane_n);

    vec3d lineToIntersect = Vector_Mul(v, t);
    return Vector_Add(lineStart, lineToIntersect);
}

// experimental alternative
vec3d Vector_IntersectPlane2(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd, float &t ) {

    // Javidx9: "this is a standard (easy to find online) algorithm, nothing clever"
    // Joost: see for an explanation: https://math.stackexchange.com/questions/2041296/algorithm-for-line-in-plane-intersection-in-3d
    plane_n = Vector_Normalise(plane_n);
    float plane_d = -Vector_DotProduct(plane_n, plane_p);

    vec3d v = Vector_Sub( lineEnd, lineStart );
    t = (-plane_d - Vector_DotProduct(lineStart, plane_n)) / Vector_DotProduct(v, plane_n);

    vec3d lineToIntersect = Vector_Mul(v, t);
    return Vector_Add(lineStart, lineToIntersect);
}

// Testing code starts here

void Vector_Print( vec3d &v1, bool end_line ) {
    std::cout << "( " <<v1.x << ", " <<v1.y << ", " <<v1.z << ", " <<v1.w << "  ) " ;
    if (end_line)
        std::cout << std::endl;
}

vec3d Vector_Get( float x, float y, float z, float w ) {
    vec3d v;
    v.x = x; v.y = y; v.z = z; v.w = w;
    return v;
}

// use for vec2d vectors - prints the contents of a vec2d to a string and returns it
std::string Vector_PrintToString2( std::string header, vec2d &v ) {

    std::string s;
    s.append( header );
    s.append( "\n");

    s.append( std::to_string( v.u ) + " " +
              std::to_string( v.v ) + " " +
              std::to_string( v.w ) + "\n" );

    return s;
}

// use for vec3d vectors - prints the contents of a vec3d to a string and returns it
std::string Vector_PrintToString3( std::string header, vec3d &v ) {

    std::string s;
    s.append( header );
//    s.append( "\n");

    s.append( std::to_string( v.x ) + " " +
              std::to_string( v.y ) + " " +
              std::to_string( v.z ) + " " +
              std::to_string( v.w ) + "\n" );

    return s;
}

