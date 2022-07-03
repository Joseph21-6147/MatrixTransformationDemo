// Demo program for matrix transforms

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include       "vec3d.h"
#include      "mat4x4.h"
#include "graphics_3D.h"

struct mesh {
    std::vector<triangle> tris;
};

// ==============================/   Game engine class    /==============================

class MatrixTransformDemo : public olc::PixelGameEngine {

public:
    MatrixTransformDemo() {
        sAppName = "MatrixTransformDemo";
    }

private:
    mesh meshCube;

    camera cam1,    // for rendering cube
           cam2;    // for displaying matrix info

    float fFoV, fNear, fFar;

    mat4x4 mTransform,   // tranformation matrix
           mValues;      // contains scaling factor, rotation angle and translation offset for (x, y, z),

// ==============================/   Rendering code    /==============================

    void RenderTriangles( std::vector<triangle> &trisToRender ) {

        switch (glbRenderMode) {
            case RM_TEXTURED:
            case RM_GREYFILLED:
                for (auto &t : trisToRender ) {
                    FillTriangle( t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, olc::Pixel( t.r, t.g, t.b ));
                }
                break;
            case RM_TEXTURED_PLUS:
            case RM_GREYFILLED_PLUS:
                for (auto &t : trisToRender ) {
                    FillTriangle( t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, olc::Pixel( t.r, t.g, t.b ));
                    DrawTriangle( t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, RM_FRAMECOL_PGE );
                }
                break;
            case RM_WIREFRAME:
                for (auto &t : trisToRender ) {
                    DrawTriangle( t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, RM_FRAMECOL_PGE );
                }
                break;
        }
    }

// ==============================/   End of rendering code    /==============================

    void DisplayMatrix( mat4x4 mTrf, mat4x4 mVal, int x, int y ) {

        // lambda convenience function for rounding floats to fixed length substring
        auto mkstr = [=]( float fValue ) -> std::string {
            return std::to_string( fValue ).substr( 0, 5 );
        };

        // display scale factors / rotation angles / translation offsets
        DrawString( x + 5, y +  30, "           x     y     z   " );
        DrawString( x + 5, y +  50, "scale:   " + mkstr( mVal.m[0][0] ) + " " + mkstr( mVal.m[0][1] ) + " " + mkstr( mVal.m[0][2] ), olc::GREY   );
        DrawString( x + 5, y +  70, "angle:   " + mkstr( mVal.m[1][0] ) + " " + mkstr( mVal.m[1][1] ) + " " + mkstr( mVal.m[1][2] ), olc::YELLOW );
        DrawString( x + 5, y +  90, "trnsl:   " + mkstr( mVal.m[2][0] ) + " " + mkstr( mVal.m[2][1] ) + " " + mkstr( mVal.m[2][2] ), olc::GREEN  );

        // display resulting transformation matrix
        DrawString( x + 15, y + 150, "Transformation matrix" );
        DrawString( x + 15, y + 170, mkstr( mTrf.m[0][0] ) + " " + mkstr( mTrf.m[0][1] ) + " " + mkstr( mTrf.m[0][2] ) + " " + mkstr( mTrf.m[0][3] ) );
        DrawString( x + 15, y + 190, mkstr( mTrf.m[1][0] ) + " " + mkstr( mTrf.m[1][1] ) + " " + mkstr( mTrf.m[1][2] ) + " " + mkstr( mTrf.m[1][3] ) );
        DrawString( x + 15, y + 210, mkstr( mTrf.m[2][0] ) + " " + mkstr( mTrf.m[2][1] ) + " " + mkstr( mTrf.m[2][2] ) + " " + mkstr( mTrf.m[2][3] ) );
        DrawString( x + 15, y + 230, mkstr( mTrf.m[3][0] ) + " " + mkstr( mTrf.m[3][1] ) + " " + mkstr( mTrf.m[3][2] ) + " " + mkstr( mTrf.m[3][3] ) );
    }

    void DisplayProjInfo( float fFieldOfView, float fNearPlane, float fFarPlane, int x, int y ) {

        // lambda convenience function for rounding floats to fixed length substring
        auto mkstr = [=]( float fValue ) -> std::string {
            return std::to_string( fValue ).substr( 0, 5 );
        };

        // display field of view and near / far plane values
        DrawString( x + 5, y +  50, "FoV:    " + mkstr( fFieldOfView ));
        DrawString( x + 5, y +  70, "Fnear:  " + mkstr( fNearPlane   ));
        DrawString( x + 5, y +  90, "Ffar:   " + mkstr( fFarPlane    ));
    }

public:
    // auxiliary function for initializing cube
    triangle make_tri( float f01, float f02, float f03, float f04,
                       float f05, float f06, float f07, float f08,
                       float f09, float f10, float f11, float f12,
                       float f13, float f14, float f15,
                       float f16, float f17, float f18,
                       float f19, float f20, float f21 ) {
        triangle t;
        t.p[0] = { f01, f02, f03, f04 };
        t.p[1] = { f05, f06, f07, f08 };
        t.p[2] = { f09, f10, f11, f12 };
        t.t[0] = { f13, f14, f15 };
        t.t[1] = { f16, f17, f18 };
        t.t[2] = { f19, f20, f21 };
        return t;
    }

    bool OnUserCreate() override {

        // create the depth buffer
        InitDepthBuffer( ScreenWidth(), ScreenHeight() );

        // Initialize the unit cube, including texturing coordinates [ which are not used in this demo :) ]
        triangle t;
        t = make_tri( 0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f ); meshCube.tris.push_back(t);    // SOUTH
        t = make_tri( 0.0f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f ); meshCube.tris.push_back(t);
        t = make_tri( 1.0f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f ); meshCube.tris.push_back(t);    // EAST
        t = make_tri( 1.0f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f ); meshCube.tris.push_back(t);
        t = make_tri( 1.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f ); meshCube.tris.push_back(t);    // NORTH
        t = make_tri( 1.0f, 0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f ); meshCube.tris.push_back(t);
        t = make_tri( 0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f ); meshCube.tris.push_back(t);    // WEST
        t = make_tri( 0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f ); meshCube.tris.push_back(t);
        t = make_tri( 0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f ); meshCube.tris.push_back(t);    // TOP
        t = make_tri( 0.0f, 1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f ); meshCube.tris.push_back(t);
        t = make_tri( 1.0f, 0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f ); meshCube.tris.push_back(t);    // BOTTOM
        t = make_tri( 1.0f, 0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f ); meshCube.tris.push_back(t);

        fFoV =  90.0f;
        fNear =  0.1f;
        fFar  = 20.0f;

        // create two camera's, the first for cube rendering, the second only as a text viewport for matrix info
        cam1.InitCamera( this, "camera 1", 0.01f * ScreenWidth(), 0.05f * ScreenHeight(), 0.54f * ScreenWidth(), 0.95f * ScreenHeight(), fFoV, fNear, fFar );
        cam2.InitCamera( this, "camera 2", 0.55f * ScreenWidth(), 0.35f * ScreenHeight(), 0.99f * ScreenWidth(), 0.95f * ScreenHeight() );

        // a little offset to put the camera close, but not on the cube
        cam1.vPosition = { 0.5f, 0.5f, -2.0f };
        cam1.RecalculateCamera();

        cam1.SetRGBrange( 32, 255 );
        glbRenderMode = RM_GREYFILLED_PLUS;

        // the mValues matrix is used for the 9 variables to be changed as input to the transform matrix:
        //     scale factor       - x, y, z
        //     rotation angle     - x, y, z
        //     translation offset - x, y, z

        // setup test matrix for displaying
        mTransform = Matrix_MakeIdentity();
        mValues = Matrix_Buildup(  1.0f, 1.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 0.0f );

        // clear the complete screen
        FillRect( 0, 0, ScreenWidth(), ScreenHeight(), olc::DARK_RED );
        // draw user instructions on screen
        DrawString( cam2.nViewPortX1 +  10, cam2.nViewPortY1 - 120, "hold Q, W, E for scale" );
        DrawString( cam2.nViewPortX1 +  10, cam2.nViewPortY1 - 100, "     A, S, D for angle" );
        DrawString( cam2.nViewPortX1 +  10, cam2.nViewPortY1 -  80, "     Z, X, C for trnsl" );
        DrawString( cam2.nViewPortX1 +  10, cam2.nViewPortY1 -  40, "change value: arrow keys" );

        DrawString( cam2.nViewPortX1 + 300, cam2.nViewPortY1 - 120, "hold V for Field of View" );
        DrawString( cam2.nViewPortX1 + 300, cam2.nViewPortY1 - 100, "     N for Near plane"    );
        DrawString( cam2.nViewPortX1 + 300, cam2.nViewPortY1 -  80, "     F for Far  plane"    );
        DrawString( cam2.nViewPortX1 + 300, cam2.nViewPortY1 -  40, "change value: + / - (num pad)" );

        return true;
    }

    bool OnUserUpdate( float fElapsedTime ) override {

        // let user choose from render modes
        if ( GetKey( olc::F1 ).bPressed ) glbRenderMode = RM_INVISIBLE      ;
        if ( GetKey( olc::F2 ).bPressed ) glbRenderMode = RM_GREYFILLED     ;
        if ( GetKey( olc::F3 ).bPressed ) glbRenderMode = RM_GREYFILLED_PLUS;
        if ( GetKey( olc::F4 ).bPressed ) glbRenderMode = RM_WIREFRAME      ;
        if ( GetKey( olc::F5 ).bPressed ) glbRenderMode = RM_WIREFRAME_RGB  ;
        if ( GetKey( olc::F6 ).bPressed ) glbRenderMode = RM_TEXTURED       ;
        if ( GetKey( olc::F7 ).bPressed ) glbRenderMode = RM_TEXTURED_PLUS  ;

        // let user make updates to the input matrix
        // let the raster qwe / asd / zxc be the activators per matrix component, and
        // alter the value with arrow keys

        int sel_index_x = -1;
        int sel_index_y = -1;

        if (GetKey( olc::Q ).bHeld) { sel_index_x = 0; sel_index_y = 0; }
        if (GetKey( olc::W ).bHeld) { sel_index_x = 1; sel_index_y = 0; }
        if (GetKey( olc::E ).bHeld) { sel_index_x = 2; sel_index_y = 0; }

        if (GetKey( olc::A ).bHeld) { sel_index_x = 0; sel_index_y = 1; }
        if (GetKey( olc::S ).bHeld) { sel_index_x = 1; sel_index_y = 1; }
        if (GetKey( olc::D ).bHeld) { sel_index_x = 2; sel_index_y = 1; }

        if (GetKey( olc::Z ).bHeld) { sel_index_x = 0; sel_index_y = 2; }
        if (GetKey( olc::X ).bHeld) { sel_index_x = 1; sel_index_y = 2; }
        if (GetKey( olc::C ).bHeld) { sel_index_x = 2; sel_index_y = 2; }

        // if any of the activator keys is held, check if the arrow keys are pressed
        if (sel_index_x >= 0 && sel_index_y >= 0) {
            if (GetKey( olc::UP   ).bReleased) mValues.m[sel_index_y][sel_index_x] = 1.0f;
            if (GetKey( olc::DOWN ).bReleased) mValues.m[sel_index_y][sel_index_x] = 0.0f;

            if (GetKey( olc::LEFT ).bHeld     ||
                GetKey( olc::LEFT ).bReleased) mValues.m[sel_index_y][sel_index_x] -= 0.5 * fElapsedTime;
            if (GetKey( olc::RIGHT).bHeld     ||
                GetKey( olc::RIGHT).bReleased) mValues.m[sel_index_y][sel_index_x] += 0.5 * fElapsedTime;
        }

        auto key_combination = [=]( olc::Key k1, olc::Key k2 ) {
            return (GetKey( k1 ).bHeld && (GetKey( k2 ).bPressed || GetKey( k2 ).bHeld));
        };

        if (key_combination( olc::V, olc::NP_ADD )) fFoV  +=  2.0f * fElapsedTime;        // adapt field of view
        if (key_combination( olc::V, olc::NP_SUB )) fFoV  -=  2.0f * fElapsedTime;
        if (key_combination( olc::N, olc::NP_ADD )) fNear +=  2.0f * fElapsedTime;        // adapt near plane
        if (key_combination( olc::N, olc::NP_SUB )) fNear -=  2.0f * fElapsedTime;
        if (key_combination( olc::F, olc::NP_ADD )) fFar  += 10.0f * fElapsedTime;        // adapt far plane
        if (key_combination( olc::F, olc::NP_SUB )) fFar  -= 10.0f * fElapsedTime;
        // update camera with new projection matrix
        cam1.UpdateCamera( fFoV, fNear, fFar );

        // create the transformation matrix with the values from the mValues matrix
        mTransform = Matrix_MakeTransformComplete( mValues.m[0][0], mValues.m[0][1], mValues.m[0][2],     // scaling x, y and z
                                                   mValues.m[1][0], mValues.m[1][1], mValues.m[1][2],     // rotation x, y and z
                                                   mValues.m[2][0], mValues.m[2][1], mValues.m[2][2] );   // translation x, y and z

        // render the cube, transformed with the input matrix
		std::vector<triangle> vecWorldCubeTris,
                              vecTrianglesToRaster,
                              vecTrianglesToRender;

        // Do transformations into world space
        for (auto triOriginal : meshCube.tris) {
            triangle triTransformed;

            camera::Tri_WorldTransform( triOriginal, mTransform, triTransformed );
            vecWorldCubeTris.push_back( triTransformed );
        }
        // Do the culling, the view and project transform per camera. The output is added
        // to the vector that is passed as parameter.
        for (auto triTransformed : vecWorldCubeTris) {
            // NOTE: clipping against near plane is done in this function.
            cam1.CullViewAndProjectTriangle( triTransformed, vecTrianglesToRaster );
        }
        // do the clipping against the borders of the viewport and produce a list to render
        cam1.RasterizeTriangles( vecTrianglesToRaster, vecTrianglesToRender );

        // Clear viewports
        cam1.ClearCameraViewPort();
        cam2.ClearCameraViewPort();

        // finally render the results
        RenderTriangles( vecTrianglesToRender );

        // display scaling, rotation and translation values and transformation matrix
        DisplayMatrix( mTransform, mValues, cam2.nViewPortX1 + 10, cam2.nViewPortY1 + 10 );

        DrawString( 10, 10, "F1 - F7: select render mode" );

        DisplayProjInfo( fFoV, fNear, fFar, cam2.nViewPortX1 + 300, cam2.nViewPortY1 + 10 );

        return true;
    }
};

// let the screen dimensions be constant and vary the resolution by adapting the pixel size
// error on startup, then lower SCREEN_X and/or SCREEN_Y
#define SCREEN_X   1280
#define SCREEN_Y    720
#define PIXEL_X       1
#define PIXEL_Y       1

int main()
{
	MatrixTransformDemo demo;
	if (demo.Construct( SCREEN_X / PIXEL_X, SCREEN_Y / PIXEL_Y, PIXEL_X, PIXEL_Y ))
		demo.Start();

	return 0;
}

