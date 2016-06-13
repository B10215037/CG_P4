#pragma once

#include <QtWidgets/QMainWindow>

/*************************************************************************
 * HMatrix hardly takes the place of a real matrix class
 * but it does make some things easier
 *************************************************************************/
typedef float HMatrix[4][4];

/*************************************************************************
 * draw a little cube centered
 * pass the size of the cube. it gets drawn at the origin
 *************************************************************************/
void drawCube(float x, float y, float z, float l);

/*************************************************************************
 * draw a ground plane
 *************************************************************************/
/*
 * draws a colored checkerboard
 */
extern float floorColor1[3];
extern float floorColor2[3];

/*
 * draw the actual ground plane
 * the groundplane is square
 * the numSquares is the number of squares in each across an edge
 */
void drawFloor(float size = 10, int nSquares = 8);

/*************************************************************************
 * it remembers what the state of the lighting was
 * and smooth shading (on or off)
 *************************************************************************/
typedef enum {
    on      =  1,
    off     = -1,
    keep    =  0
} LightOnOff;

void setLighting(const LightOnOff lighting=keep,
                 const LightOnOff smooth=keep);

/*
 * pop last state off the stack
 */
void restoreLighting();

/*************************************************************************
 * code for doing shadows with the stencil buffer and blending
 * these routines change the opengl state
 * and may not return it to where you want it to be
 * (a better implementation would save state)
 * to get nice looking shadows
 * this uses alpha blending and the stencil buffer
 * which means you must declare them when you create the window:
 * mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );
 *************************************************************************/
/*
 * before drawing the floor
 * setup the stencil buffer and enable depth testing
 */
void setupFloor();

/*
 * once you're done drawing the floor
 * setup to draw the objects regularly
 */
void setupObjects();

/*
 * Set up the projection matrix to project the shadow onto the floor
 */
void setupShadows();

/*
 * Set it back to original projection matrix
 */
void unsetupShadows();

/*
 * r1,r2 are two points on the line
 * l is the initial position of the point
 * r is the resulting position
 * r will share 1 of its coordinates with l
 * but will be on the line
 */
void mousePoleGo(double r1x, double r1y, double r1z,
                 double r2x, double r2y, double r2z,
                 double lx, double ly, double lz,
                 double &rx, double &ry, double &rz,
                 bool elevator);

int mouseMoveEvent(QMouseEvent *e);

/*************************************************************************
 * useful math
 *************************************************************************/
float radiansToDegrees(const float);
