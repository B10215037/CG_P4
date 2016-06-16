#ifndef MODEL_H
#define MODEL_H

#include <QStack>
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QtOpenGL/QtOpenGL>
#include <math.h>
#include "Pnt3f.H"

enum LightType {
    none, point, spot, directional
};

struct Light {
    int index = -1;

    QVector4D position, color = QVector4D(1, 1, 1, 1);
    float power = 500;
    LightType type = none;

    // not use for point light
    QVector4D orientation =  QVector4D(0, -1, 0, 1); // from position
    float spot_angle = 10; // one side
};

struct ObjPart {
    QString usemtl_name;
    int texture_id = -1, v_size = 0;
    QVector3D kd;
};

class Model
{
public:
    Model() {
        pos = Pnt3f();
        rotation = Pnt3f();
        scale = Pnt3f(1, 1, 1);
    }
    Model(const QString &file_path);
    ~Model() {
        for (int i=0; i < textures.size(); ++i)
            delete textures[i];
    }

    static void initShader(QString vertex_shader_path, QString fragment_shader_path);
    static void deleteShader();
    void initVAO_VBO();

    /*************************************************************************
     * openGL use column-major matrix
     * but it transpose 2 times
     * so just view as row-major matrix
     *************************************************************************/
    void dimension16To4X4(float source[], float target[][4]);

    void render();

    void loadMTL(const QString &mtl_path);
    int loadTexture(const QString &texture_path);

    static QOpenGLShaderProgram *shader_program;
    static QOpenGLShader *vertex_shader, *fragment_shader;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer v_vbo, vt_vbo, vn_vbo;
    QStack<QVector3D> v, vn;
    QStack<QVector2D> vt;
    QStack<QOpenGLTexture*> textures;
    QVector4D
        mtl_ambient = QVector4D(.3f, .3f, .3f, 1),
        mtl_specular = QVector4D(.3f, .3f, .3f, 1);
    Light light;
    static QStack<Light*> lights;

    QString file_path, file_name;
    Pnt3f pos, rotation, scale;
    float rotation_matrix[4][4];
    bool is_selected = false, use_rotation_matrix = false, use_lighting = true;
    QStack<ObjPart> obj_parts;
};

#endif
