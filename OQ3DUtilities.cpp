#define _USE_MATH_DEFINES

#include <math.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include "OQ3DUtilities.h"

using std::vector;

/*************************************************************************
 * draw a little cube at the origin
 * use a transform to put it in the appropriate place
 * pass the size of the cube
 * rather than using a scale transform
 * since we want our normals to stay unit length if possible
 *************************************************************************/
void drawCube(float x, float y, float z, float l)
{
    glPushMatrix();
        glTranslated(x,y,z);
        glScalef(l,l,l);
        glBegin(GL_QUADS);
            glNormal3d( 0,0,1);
            glVertex3d( 0.5, 0.5, 0.5);
            glVertex3d(-0.5, 0.5, 0.5);
            glVertex3d(-0.5,-0.5, 0.5);
            glVertex3d( 0.5,-0.5, 0.5);

            glNormal3d( 0, 0, -1);
            glVertex3d( 0.5, 0.5, -0.5);
            glVertex3d( 0.5,-0.5, -0.5);
            glVertex3d(-0.5,-0.5, -0.5);
            glVertex3d(-0.5, 0.5, -0.5);

            glNormal3d( 0, 1, 0);
            glVertex3d( 0.5, 0.5, 0.5);
            glVertex3d( 0.5, 0.5,-0.5);
            glVertex3d(-0.5, 0.5,-0.5);
            glVertex3d(-0.5, 0.5, 0.5);

            glNormal3d( 0,-1,0);
            glVertex3d( 0.5,-0.5, 0.5);
            glVertex3d(-0.5,-0.5, 0.5);
            glVertex3d(-0.5,-0.5,-0.5);
            glVertex3d( 0.5,-0.5,-0.5);

            glNormal3d( 1,0,0);
            glVertex3d( 0.5, 0.5, 0.5);
            glVertex3d( 0.5,-0.5, 0.5);
            glVertex3d( 0.5,-0.5,-0.5);
            glVertex3d( 0.5, 0.5,-0.5);

            glNormal3d(-1,0,0);
            glVertex3d(-0.5, 0.5, 0.5);
            glVertex3d(-0.5, 0.5,-0.5);
            glVertex3d(-0.5,-0.5,-0.5);
            glVertex3d(-0.5,-0.5, 0.5);
        glEnd();
    glPopMatrix();
}

/*************************************************************************
 * the two colors for the floor for the check board
 *************************************************************************/
float floorColor1[3] = { .7f, .7f, .7f }; // Light color
float floorColor2[3] = { .3f, .3f, .3f }; // Dark color

/*
 * Draw the check board floor without texturing it
 */
void drawFloor(float size, int nSquares)
{
    // parameters:
    float maxX = size/2, maxY = size/2;
    float minX = -size/2, minY = -size/2;

    int x,y,v[3],i;
    float xp,yp,xd,yd;
    v[2] = 0;
    xd = (maxX - minX) / ((float) nSquares);
    yd = (maxY - minY) / ((float) nSquares);
    glBegin(GL_QUADS);
    for(x=0,xp=minX; x<nSquares; x++,xp+=xd) {
        for(y=0,yp=minY,i=x; y<nSquares; y++,i++,yp+=yd) {
            glColor4fv(i%2==1 ? floorColor1:floorColor2);
            glNormal3f(0, 1, 0);
            glVertex3d(xp,      0, yp);
            glVertex3d(xp,      0, yp + yd);
            glVertex3d(xp + xd, 0, yp + yd);
            glVertex3d(xp + xd, 0, yp);

        } // end of for j
    }// end of for i
    glEnd();
}

/*************************************************************************
 * save the lighting state on the stack so it can be restored
 *************************************************************************/
struct LightState {
    bool lighting;
    bool smooth;

    LightState(bool l, bool s) : lighting(l), smooth(s) {}
};
static vector<LightState> lightStateStack;

void setLighting(const LightOnOff lighting, const LightOnOff smoothi)
{
    int smooth;
    bool lights = glIsEnabled(GL_LIGHTING) > 0;
    glGetIntegerv(GL_SHADE_MODEL, &smooth);
    lightStateStack.push_back(LightState( lights, (smooth == GL_SMOOTH) ));

    if (lighting != keep) {
        if (lighting == on) glEnable(GL_LIGHTING);
        else glDisable(GL_LIGHTING);
    }
    if (smoothi != keep) {
        if (smoothi == on) glShadeModel(GL_SMOOTH);
        else glShadeModel(GL_FLAT);
    }
}

void restoreLighting()
{
    if (!lightStateStack.empty()) {
        if ((lightStateStack.end() - 1)->lighting)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);

        if ((lightStateStack.end() - 1)->smooth)
            glShadeModel(GL_SMOOTH);
        else
            glShadeModel(GL_FLAT);
    }
}

/*************************************************************************
 * Shadows
 * before drawing the floor
 * setup the stencil buffer and enable depth testing
 * when we draw the ground plane
 * set the stencil buffer to 1
 * draw shadows where the stencil buffer is one
 *************************************************************************/
void setupFloor()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS,0x1,0x1);
    glStencilOp(GL_REPLACE,GL_REPLACE,GL_REPLACE);
    glStencilMask(0x1);		// only deal with the 1st bit
}

/*
 * now draw the objects normally
 * write the stencil to show the floor isn't there anymore
 */
void setupObjects()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS,0x0,0x0);
    glStencilOp(GL_REPLACE,GL_REPLACE,GL_REPLACE);
    glStencilMask(0x1);		// only deal with the 1st bit
}

/*
 * "hack" shadows
 * just squish the objects onto the floor
 * a projection matrix to do the squishing
 * basically set the Y component to zero
 * turn off lighting since we want the objects to be black
 * draw the shadows as transparent to make things look nice
 * turn off the Z-Buffer to avoid Z-Fighting
 * this means we'll see the shadows through the floor
 */
void setupShadows()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL,0x1,0x1);
    glStencilOp(GL_KEEP,GL_ZERO,GL_ZERO);
    glStencilMask(0x1);		// only deal with the 1st bit

    glPushMatrix();
    // a matrix that squishes things onto the floor
    float sm[16] = {1,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,1};
    glMultMatrixf(sm);
    // draw in transparent black (to dim the floor)
    glColor4f(0,0,0,.5);
}

/*
 * puts things back to a "normal" state
 */
void unsetupShadows()
{
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
}

/*************************************************************************
 * given a pair of points on the mouse ray
 * it assumes the plane parallel to the ground
 * unless the elevator button is pushed
 * in which case it is the YZ plane
 *************************************************************************/
void mousePoleGo(double r1x, double r1y, double r1z,
                 double r2x, double r2y, double r2z,
                 double lx, double ly, double lz,
                 double &rx, double &ry, double &rz,
                 bool elevator)
{
    rx = lx; ry = ly; rz = lz;
    if (elevator || (fabs(r1y - r2y) < .01)) {
        if (fabs(r1z - r2z) > fabs(r1x - r2x)) {
            double zd = r1z - r2z;
            if (fabs(zd) > .01) {
                double zp = (lz - r1z) / zd;
                rx = r1x + (r1x - r2x) * zp;
                ry = r1y + (r1y - r2y) * zp;
            }
        }
        else {
            double xd = r1x - r2x;
            if (fabs(xd) > .01) {
                double xp = (lx - r1x) / xd;
                rz = r1z + (r1z - r2z) * xp;
                ry = r1y + (r1y - r2y) * xp;
            }
        }
    }
    else {
        double yd = r1y - r2y;
        // we have already made sure that the elevator is not singular
        double yp = (ly - r1y) / yd;
        rx = r1x + (r1x - r2x) * yp;
        rz = r1z + (r1z - r2z) * yp;
    }
}

/*************************************************************************
 * useful math
 *************************************************************************/
static const float rtdf = static_cast<float>(180.0 / M_PI);
float radiansToDegrees(const float x)
{
    return rtdf * x;
}
