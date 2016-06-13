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
    point, spot
};

struct Light {
    QVector4D position, color = QVector4D(1, 1, 1, 1);
    float power = 5000;
    LightType type = point;

    QVector4D spot_direction =  QVector4D(0, -1, 0, 1); // from position
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
        if (vertex_shader) delete vertex_shader;
        if (fragment_shader) delete fragment_shader;
        if (shader_program) delete shader_program;
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
    static Light light1, light2;

    QString file_path, file_name;
    Pnt3f pos, rotation, scale;
    float rotation_matrix[4][4];
    bool is_selected = false, use_rotation_matrix = false, use_lighting = true;
    QStack<ObjPart> obj_parts;

private:
    void loadMTL(const QString &mtl_path);
    int loadTexture(const QString &texture_path);
};

#endif
