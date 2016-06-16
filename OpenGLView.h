#ifndef OPENGLVIEW_H
#define OPENGLVIEW_H

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#include <QGLWidget>
#include <GL/GLU.h>
#include <QtGui/QtGui>
#include <QtGui/QOpenGLFunctions_4_0_Core>
#include <QStack>
#include "OQ3DUtilities.h"
#include "ArcBallCam.H"
#include "Track.H"
#include "Model.H"

class OpenGLView : public QGLWidget, protected QOpenGLFunctions_4_0_Core
{
    Q_OBJECT

public:
    explicit OpenGLView(QWidget *parent = 0);

    virtual void initializeGL();
    virtual void paintGL();

    void setProjection();

    /*************************************************************************
     * all of the actual drawing happens in this routine
     *************************************************************************/
    void drawStuff(bool doingShadows=false);

    void resetArcball();

    /*************************************************************************
     * pick a point (for when the mouse goes down)
     *************************************************************************/
    void doPick(int mx, int my);

    /*************************************************************************
     * track mode:
     * 0: line
     * 1: track
     * 2: road
     *
     * curve mode:
     * 0: linear
     * 1: cardinal
     * 2: cubic
     *************************************************************************/
    void drawTrack(bool doingShadows);

    /*
     * compute and save track points
     * linear curve, cardinal curve, cubic curve
     */
    void computeTrack();

    /*
     * cardinal curve, cubic curve
     */
    void computeSpline(const HMatrix M);

    /*************************************************************************
     * draw train that run on the track
     * track_points' orient means up vector not tangent vector
     * use histogram like CDF to let the train's velocity be same
     *************************************************************************/
    void runTrain();

    // keep an ArcBall for the UI
    ArcBallCam      arcball;
    // simple - just remember which cube is selected
    int             selectedCube[2];
    // The track of the entire scene
    QStack<CTrack>	m_pTrack;

    int camera;
    int curve;
    int track;
    bool
    isrun = false,
    add_support_structure = false,
    useFBO = false,
    normal_view = false;
    float t_time = 0;
    const int STD_DIVIDE_LINE = 10000; // standard distance: 50
    float train_speed = 0;

    /*************************************************************************
     * record track points that don't need to compute when track is fixed
     *************************************************************************/
    class Pnt3_data {
    public:
        Pnt3_data() {
            pos = Pnt3f();
            orient = Pnt3f();
        }
        Pnt3_data(Pnt3f pos, Pnt3f orient) :pos(pos), orient(orient) {}
        Pnt3f pos, orient;
    };
    QStack<Pnt3_data> track_points;

    /*************************************************************************
     * trains_index is objs' index that is defined as train(roller coaster)
     *************************************************************************/
    QStack<Model*> objs;
    QStack<int> trains_index;
    int selected_obj = -1;
    Model *skybox, *mountain, *city_island;

    Pnt3f train_view_eye, train_view_center, train_view_up;

    GLuint shaderProgram;
    GLuint VAO;
    GLuint framebuffer;
    GLuint textureColorbuffer;
    GLuint shadow_maps[4];

    void initShader(QString vertex_shader_path, QString fragment_shader_path);
};

#endif // OPENGLVIEW_H
