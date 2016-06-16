#include "Model.H"

QOpenGLShaderProgram *Model::shader_program;
QOpenGLShader *Model::vertex_shader, *Model::fragment_shader;
QStack<Light*> Model::lights;

Model::Model(const QString &file_path)
    : file_path(file_path), file_name(QFileInfo(file_path).fileName())
{
    if (!file_path.isEmpty()) {

        QFile file(file_path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

            initShader("../Resources/Shaders/model/vetex", "../Resources/Shaders/model/fragment");

            int part_index = -1;
            bool add_part = true;
            QStack<QVector2D> temp_vt;
            QStack<QVector3D> temp_v, temp_vn;
            QStack<uint> temp_v_indices, temp_vt_indices, temp_vn_indices;

            QTextStream file_text(&file);
            while (!file_text.atEnd()) {

                QString file_line = file_text.readLine();
                QStringList segment = file_line.split(" ");

                if (file_line.startsWith("v ")) {
                    QVector3D v;
                    v.setX(segment[1].toFloat());
                    v.setY(segment[2].toFloat());
                    v.setZ(segment[3].toFloat());
                    temp_v.push(v);
                }
                else if (file_line.startsWith("vt ")) {
                    QVector2D vt;
                    vt.setX(segment[1].toFloat());
                    vt.setY(segment[2].toFloat());
                    temp_vt.push(vt);
                }
                else if (file_line.startsWith("vn ")) {
                    QVector3D vn;
                    vn.setX(segment[1].toFloat());
                    vn.setY(segment[2].toFloat());
                    vn.setZ(segment[3].toFloat());
                    temp_vn.push(vn);
                }
                else if (file_line.startsWith("usemtl ")) {
                    if (add_part) {
                        ObjPart part;
                        obj_parts.push(part);
                        ++part_index;
                        add_part = false;
                    }
                    obj_parts[part_index].usemtl_name = segment[1];
                }
                else if (file_line.startsWith("f ")) {
                    for (int i=1; i < segment.size(); i++) {
                        QStringList arg = segment[i].split("/");

                        int arg0 = arg[0].toInt();
                        if (arg0 > 0) --arg0;

                        obj_parts[part_index].v_size++;

                        //f is v/vt
                        if (arg.size() == 2) {
                            int arg1 = arg[1].toInt();
                            if (arg1 > 0) --arg1;
                            else arg1 += temp_vt.size();

                            temp_vt_indices.push(arg1);
                        }
                        // f is v/vt/vn or v//vn
                        else if (arg.size() == 3) {
                            bool ok = false;
                            int
                                    arg1 = arg[1].toInt(&ok),
                                    arg2 = arg[2].toInt();
                            if (ok) {
                                if (arg1 > 0) --arg1;
                                else arg1 += temp_vt.size();

                                temp_vt_indices.push(arg1);
                            }

                            if (arg2 > 0) --arg2;
                            else arg2 += temp_vn.size();

                            temp_vn_indices.push(arg2);
                        }
                        temp_v_indices.push(arg0);
                    }

                    add_part = true;
                }
            }

            for (int i=0; i < temp_v_indices.size(); ++i) {
                if (temp_v_indices.size())
                    v.push(temp_v[temp_v_indices[i]]);
                if (temp_vt_indices.size())
                    vt.push(temp_vt[temp_vt_indices[i]]);
                if (temp_vn_indices.size())
                    vn.push(temp_vn[temp_vn_indices[i]]);
            }
            initVAO_VBO();

            QString fp = file_path;
            loadMTL(fp.replace(".obj", ".mtl"));
        }
        file.close();
    }

    pos = Pnt3f();
    rotation = Pnt3f();
    scale = Pnt3f(1, 1, 1);
}

void Model::initShader(QString vertex_shader_path, QString fragment_shader_path)
{
    shader_program = new QOpenGLShaderProgram();

    QFileInfo vertex_shader_file_info(vertex_shader_path);
    if (vertex_shader_file_info.exists()) {
        vertex_shader = new QOpenGLShader(QOpenGLShader::Vertex);
        if (vertex_shader->compileSourceFile(vertex_shader_path))
            shader_program->addShader(vertex_shader);
        else qWarning() << "Vertex shader error: " << vertex_shader->log();
    }

    QFileInfo fragment_shader_file_info(fragment_shader_path);
    if (fragment_shader_file_info.exists()) {
        fragment_shader = new QOpenGLShader(QOpenGLShader::Fragment);
        if (fragment_shader->compileSourceFile(fragment_shader_path))
            shader_program->addShader(fragment_shader);
        else qWarning() << "Fragment shader error: " << fragment_shader->log();
    }

    shader_program->link();
}

void Model::deleteShader()
{
    if (vertex_shader) delete vertex_shader;
    if (fragment_shader) delete fragment_shader;
    if (shader_program) delete shader_program;
}

void Model::initVAO_VBO()
{
    vao.create();
    vao.bind();

    shader_program->bind();

    v_vbo.create();
    v_vbo.bind();
    v_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    v_vbo.allocate(v.constData(), v.size() * sizeof(QVector3D));
    shader_program->enableAttributeArray("v");
    shader_program->setAttributeArray("v", GL_FLOAT, 0, 3, NULL);
//    v_vbo.release();

    vt_vbo.create();
    vt_vbo.bind();
    vt_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vt_vbo.allocate(vt.constData(), vt.size() * sizeof(QVector2D));
    shader_program->enableAttributeArray("vt");
    shader_program->setAttributeArray("vt", GL_FLOAT, 0, 2, NULL);
//    vt_vbo.release();

    vn_vbo.create();
    vn_vbo.bind();
    vn_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vn_vbo.allocate(vn.constData(), vn.size() * sizeof(QVector3D));
    shader_program->enableAttributeArray("vn");
    shader_program->setAttributeArray("vn", GL_FLOAT, 0, 3, NULL);
//    vn_vbo.release();

//    shader_program->release();
//    vao.release();
}

/*************************************************************************
 * openGL use column-major matrix
 * but it transpose 2 times
 * so just view as row-major matrix
 *************************************************************************/
void Model::dimension16To4X4(float source[], float target[][4])
{
    for (int i=0; i < 16; ++i) target[i / 4][i % 4] = source[i];
}

void Model::render()
{
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    float model_view_matrix[16], projection_matrix[16], mv_matrix[4][4], p_matrix[4][4];
    glGetFloatv(GL_MODELVIEW_MATRIX, model_view_matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, projection_matrix);

    dimension16To4X4(model_view_matrix, mv_matrix);
    dimension16To4X4(projection_matrix, p_matrix);

    shader_program->bind();
    shader_program->setUniformValue("mv_matrix", mv_matrix);
    shader_program->setUniformValue("p_matrix", p_matrix);

    float
            translation_matrix[4][4] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        pos.x, pos.y, pos.z, 1
    },
            scale_matrix[4][4] = {
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    };

    shader_program->setUniformValue("translation_matrix", translation_matrix);
    if (use_rotation_matrix) {
        shader_program->setUniformValue("rotation_matrix", rotation_matrix);
        use_rotation_matrix = false;
        shader_program->setUniformValue("use_rotation_matrix", true);
    }
    else shader_program->setUniformValue("use_rotation_matrix", false);
    shader_program->setUniformValue("x_angle", rotation.x / 180 * (float)M_PI);
    shader_program->setUniformValue("y_angle", rotation.y / 180 * (float)M_PI);
    shader_program->setUniformValue("z_angle", rotation.z / 180 * (float)M_PI);
    shader_program->setUniformValue("scale_matrix", scale_matrix);

    shader_program->setUniformValue("lights_size", lights.size());
    if (lights.size() > 0) {
        QStack<QVector4D>
                lights_position,
                lights_color,
                lights_orientation;
        QStack<GLfloat> lights_power, cutoffs;
        QStack<GLint> lights_type;

        for (int i=0; i < lights.size(); ++i) {
            lights_position.push(lights[i]->position);
            lights_color.push(lights[i]->color);
            lights_power.push(lights[i]->power);
            lights_type.push(lights[i]->type);
            lights_orientation.push(lights[i]->orientation);
            cutoffs.push(cos(lights[i]->spot_angle / 180 * M_PI));
        }

        shader_program->setUniformValueArray("lights_position", lights_position.data(), lights.size());
        shader_program->setUniformValueArray("lights_color", lights_color.data(), lights.size());
        shader_program->setUniformValueArray("lights_power", lights_power.data(), lights.size(), 1);
        shader_program->setUniformValueArray("lights_type", lights_type.data(), lights.size());
        shader_program->setUniformValueArray("lights_orientation", lights_orientation.data(), lights.size());
        shader_program->setUniformValueArray("cutoffs", cutoffs.constData(), lights.size(), 1);
    }

    shader_program->setUniformValue("mtl_ambient", mtl_ambient);
    shader_program->setUniformValue("mtl_specular", mtl_specular);
    shader_program->setUniformValue("use_lighting", use_lighting);

    vao.bind();
    for (int i=0, j=0; i < obj_parts.size(); i++) {
        QVector4D kd;
        if (is_selected)
            kd = QVector4D(
                        obj_parts[i].kd.x() + .2f,
                        obj_parts[i].kd.y() + .2f,
                        obj_parts[i].kd.z() + .2f,
                        .1f);
        else
            kd = QVector4D(
                        obj_parts[i].kd.x(),
                        obj_parts[i].kd.y(),
                        obj_parts[i].kd.z(),
                        1);
        shader_program->setUniformValue("kd", kd);

        int texture_id = obj_parts[i].texture_id;
        if (texture_id >= 0) {
            textures[texture_id]->bind();
            shader_program->setUniformValue("texture", 0);
            shader_program->setUniformValue("use_texture", true);
        }
        else shader_program->setUniformValue("use_texture", false);

        glDrawArrays(GL_TRIANGLES, j, obj_parts[i].v_size);
        j+= obj_parts[i].v_size;
    }
    vao.release();
    shader_program->release();

    glDisable(GL_DEPTH_TEST);
}

void Model::loadMTL(const QString &mtl_path)
{
    if (!mtl_path.isEmpty()) {
        QFile file(mtl_path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

            QString newmtl,
                    mtl_file_name = QFileInfo(mtl_path).fileName();

            QTextStream fileText(&file);
            while (!fileText.atEnd()) {
                QString file_line = fileText.readLine();
                QStringList line_list = file_line.split(" ");

                if (file_line.startsWith("newmtl ")) {
                    newmtl = line_list[1];
                }
                else if (file_line.startsWith("Kd ")) {
                    for (int i=0; i < obj_parts.size(); ++i) {
                        if (obj_parts[i].usemtl_name == newmtl) {
                            obj_parts[i].kd.setX(line_list[1].toFloat());
                            obj_parts[i].kd.setY(line_list[2].toFloat());
                            obj_parts[i].kd.setZ(line_list[3].toFloat());
                        }
                    }
                }
                else if (file_line.startsWith("map_Kd ")) {
                    QString texture_name = line_list[1],
                            mp = mtl_path;
                    int id = loadTexture(mp.replace(mtl_file_name,
                                                     texture_name));
                    for (int i=0; i < obj_parts.size(); ++i)
                        if (obj_parts[i].usemtl_name == newmtl)
                            obj_parts[i].texture_id = id;
                }
            }
        }
        file.close();
    }
}

int Model::loadTexture(const QString &texture_path)
{
    QImage texture_image = QImage(texture_path);
    if (texture_image.isNull()) return -1;

    QOpenGLTexture *texture = new QOpenGLTexture(texture_image.mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::Repeat);

    textures.push(texture);

    return textures.size() - 1;
}
