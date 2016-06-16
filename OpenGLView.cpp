#include "OpenGLView.h"

OpenGLView::OpenGLView(QWidget *parent) :
    QGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    resetArcball();

    selectedCube[0] = -1;
    selectedCube[1] = -1;

    m_pTrack.push(CTrack());
}

void OpenGLView::initializeGL()
{
    initializeOpenGLFunctions();
    computeTrack();

    skybox = new Model("../Resources/Models/skybox/skybox2.obj");
    skybox->scale = Pnt3f(50000, 50000, 50000);
    skybox->mtl_ambient = QVector4D(1, 1, 1, 1);
    skybox->use_lighting = false;

    mountain = new Model("../Resources/Models/mountain/mountain.obj");
    mountain->pos = Pnt3f(0, 100, 0);
    mountain->scale = Pnt3f(100, 150, 100);

    city_island = new Model(
                "../Resources/Models/scifi tropical city/Sci-fi Tropical city island2.obj");
    city_island->pos = Pnt3f(100, 100, 100);
    city_island->rotation = Pnt3f(0, 90, 0);
    city_island->scale = Pnt3f(10, 10, 10);

    initShader("../Resources/Shaders/frame/vetex_framebuffer", "../Resources/Shaders/frame/fragment_framebuffer");

    GLfloat v[] = {
        1, 1, 0,    1, 1,
        1, -1, 0,   1, 0,
        -1, -1, 0,  0, 0,
        -1, 1, 0,   0, 1
    };
    GLuint indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT,GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // use frame buffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Generate texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width(), height());
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        qWarning() << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
}

void OpenGLView::initShader(QString vertex_shader_path, QString fragment_shader_path)
{
    GLuint vertexShader;
    QFile vertexShader_file(vertex_shader_path);
    if (vertexShader_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream fileText(&vertexShader_file);

        std::string str = "";
        while (!fileText.atEnd())
            str += fileText.readAll().toStdString();

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char *c_str = str.c_str();
        glShaderSource(vertexShader, 1, &c_str, NULL);
        glCompileShader(vertexShader);
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            qWarning() << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
        }
    }

    GLuint fragmentShader;
    QFile fragmentShader_file(fragment_shader_path);
    if (fragmentShader_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream fileText(&fragmentShader_file);

        std::string str = "";
        while (!fileText.atEnd())
            str += fileText.readAll().toStdString();

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *c_str = str.c_str();
        glShaderSource(fragmentShader, 1, &c_str, NULL);
        glCompileShader(fragmentShader);
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            qWarning() << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << "\n";
        }
    }

    glDeleteProgram(shaderProgram);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        qWarning() << "ERROR::SHADER_PROGRAM::LINK_FAILED\n" << infoLog << "\n";
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void OpenGLView::paintGL()
{
    // First pass
    if (useFBO)
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /*************************************************************************
     * Set up basic opengl informaiton
     *************************************************************************/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, width(), height());
    // clear the window, be sure to clear the Z-Buffer too
    glClearColor(.5f, .5f, .5f, 0);

    glEnable(GL_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    if (normal_view) {
        // prepare for projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        setProjection();		// put the code to set up matrices here
    }
    else {

        for (int i=0; i < std::max(Model::lights.size(), 4); ++i) {
            glMatrixMode(GL_PROJECTION);
            gluPerspective(120, 1, 1, 500000);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            gluLookAt(Model::lights[i]->position.x(),
                      Model::lights[i]->position.y(),
                      Model::lights[i]->position.z(),
                      Model::lights[i]->position.x() + Model::lights[i]->orientation.x(),
                      Model::lights[i]->position.y() + Model::lights[i]->orientation.y(),
                      Model::lights[i]->position.z() + Model::lights[i]->orientation.z(),
                      0, 1, 0);
            update();

            glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glBindTexture(GL_TEXTURE_2D, shadow_maps[i]);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture"), 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    glPushMatrix();
    drawStuff();
    glPopMatrix();

    // Second pass
    if (useFBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture"), 0);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void OpenGLView::setProjection()
{
    // Compute the aspect ratio (we'll need it)
    float aspect = static_cast<float>(width()) / static_cast<float>(height());

    // Check whether we use the world camp
    if (camera == 0) {
        arcball.setProjection(false);
        update();
    // Or we use the top cam
    }
    else if (camera == 1) {
        float wi, he;
        if (aspect >= 1) {
            wi = 550;
            he = wi / aspect;
        }
        else {
            he = 550;
            wi = he * aspect;
        }

        // Set up the top camera drop mode to be orthogonal and set
        // up proper projection matrix
        glMatrixMode(GL_PROJECTION);
        glOrtho(-wi, wi, -he, he, 200, -500000);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-90,1,0,0);
        update();
    }

    /// TODO:
    /// put code for train view projection here!

    else if (camera == 2) {
        glMatrixMode(GL_PROJECTION);
        gluPerspective(120, 1, 1, 500000);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(train_view_eye.x, train_view_eye.y, train_view_eye.z,
                  train_view_center.x,
                  train_view_center.y,
                  train_view_center.z,
                  train_view_up.x, train_view_up.y, train_view_up.z);
        update();
    }
}

/*************************************************************************
 * if you're drawing shadows, DO NOT set colors
 *************************************************************************/
/// TODO:
/// if you have other objects in the world, make sure to draw them
void OpenGLView::drawStuff(bool doingShadows)
{
    // Draw the control points
    // don't draw the control points if you're driving
    // (otherwise you get sea-sick as you drive through them)
    if (camera != 2) {
        for (int ti=0; ti < m_pTrack.size(); ++ti) {
            if (!m_pTrack[ti].hide_control_points) {
                for(size_t i = 0; i < m_pTrack[ti].points.size(); ++i) {
                    if (!doingShadows) {
                        if (ti != selectedCube[0] ||
                                (ti == selectedCube[0] &&
                                 ((int) i) != selectedCube[1]))
                            glColor3ub(240, 60, 60);
                        else
                            glColor3ub(240, 240, 30);
                    }
                    m_pTrack[ti].points[i].draw();
                }
                update();
            }
        }
    }

    /// TODO:
    /// draw the track
    /// call your own track drawing code
    drawTrack(doingShadows);

    /// TODO:
    /// draw the train
    ///	call your own train drawing code
    if (isrun) runTrain();
    for (int i=0; i < objs.size(); ++i)
        objs[i]->render();

    skybox->render();
    mountain->render();
    city_island->render();
}

void OpenGLView::resetArcball()
{
    // Set up the camera to look at the world
    // these parameters might seem magical, and they kindof are
    // a little trial and error goes a long way
    arcball.setup(this, 40, 250, .2f, .4f, 0);
}

void OpenGLView::doPick(int mx, int my)
{
    // since we'll need to do some GL stuff so we make this window as
    // active window
    makeCurrent();

    // get the viewport - most reliable way to turn mouse coords into GL coords
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPickMatrix((double)mx, (double)(viewport[3] - my), 5, 5, viewport);

    // now set up the projection
    setProjection();

    // now draw the objects - but really only see what we hit
    GLuint buf[100];
    glSelectBuffer(100,buf);
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(0);

    // draw the cubes, loading the names as we go
    size_t i=0;
    int all_points_size = 0;
    for (int ti=0; ti < m_pTrack.size(); ++ti) {
        all_points_size += (int)m_pTrack[ti].points.size();
        for (uint j=0; j < m_pTrack[ti].points.size(); ++i, ++j) {
            glLoadName((GLuint) (i+1));
            m_pTrack[ti].points[j].draw();
        }
    }
//    for (int j=0; j < objs.size(); ++i, ++j) {
//        glLoadName((GLuint) (i+1));
//        objs[j]->render();
//    }

    // go back to drawing mode, and see how picking did
    int hits = glRenderMode(GL_RENDER);
    if (hits) {
        // warning; this just grabs the first object hit - if there
        // are multiple objects, you really want to pick the closest
        // one - see the OpenGL manual
        // remember: we load names that are one more than the index
        int selected_name = buf[3]-1;
        if (selected_name < all_points_size) {
            for (int k=0; k < m_pTrack.size(); ++k) {
                if (selected_name < (int)m_pTrack[k].points.size()) {
                    selectedCube[0] = k;
                    selectedCube[1] = selected_name;
                    break;
                }
                else
                    selected_name -= (int)m_pTrack[k].points.size();
            }
//            selected_obj = -1;
//            for (int j=0; j < objs.size(); ++j)
//                objs[j]->is_selected = false;
        }
        else {
            selectedCube[0] = -1;
            selectedCube[1] = -1;
//            selected_obj = selected_name - all_points_size;
//            for (int j=0; j < objs.size(); ++j)
//                if (j == selected_obj)
//                    objs[j]->is_selected = true;
//                else
//                    objs[j]->is_selected = false;
        }
    }
    else { // nothing hit, nothing selected
        selectedCube[0] = -1;
        selectedCube[1] = -1;
//        selected_obj = -1;
//        for (int i=0; i < objs.size(); ++i)
//            objs[i]->is_selected = false;
    }
}

/*************************************************************************
 * track mode:
 * 0: line
 * 1: track
 * 2: road
 * 3: hide all (switch case default + implement here and in drawStuff())
 *
 * curve mode:
 * 0: linear
 * 1: cardinal
 * 2: cubic
 *************************************************************************/
void OpenGLView::drawTrack(bool doingShadows)
{
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
//    glEnable(GL_LIGHT1);
    glPushMatrix();

    switch (track) {
    case 1: // normal track
        for (int i=0; i < track_points.size(); i+=500) {
            Pnt3f
                    ft0 = track_points[i].pos,
                    ft1 = track_points[(i+250) % track_points.size()].pos,
                    ot0 = track_points[i].orient,
                    cross;
            // cross
            cross = (ft1 - ft0) * ot0;
            cross.normalize();
            cross = cross * 5;

            glBegin(GL_QUADS);
            if ( !doingShadows ) glColor3ub(78 , 36 , 13);
            glVertex3f(ft0.x + cross.x, ft0.y + cross.y, ft0.z + cross.z);
            glVertex3f(ft1.x + cross.x, ft1.y + cross.y, ft1.z + cross.z);
            glVertex3f(ft1.x - cross.x, ft1.y - cross.y, ft1.z - cross.z);
            glVertex3f(ft0.x - cross.x, ft0.y - cross.y, ft0.z - cross.z);
            glEnd();
        }
    case 0: // line track
        for (int i=0; i < track_points.size(); ++i) {
            Pnt3f
                    ft0 = track_points[i].pos,
                    ft1 = track_points[(i+1) % track_points.size()].pos,
                    ot0 = track_points[i].orient,
                    cross;
            // cross
            cross = (ft1 - ft0) * ot0;
            cross.normalize();
            cross = cross * 2.5f;

            glLineWidth(10);
            glBegin(GL_LINES);
            if ( !doingShadows ) glColor3ub(32 , 32 , 64);
            glVertex3f(ft0.x + cross.x, ft0.y + cross.y, ft0.z + cross.z);
            glVertex3f(ft1.x + cross.x, ft1.y + cross.y, ft1.z + cross.z);

            glVertex3f(ft0.x - cross.x, ft0.y - cross.y, ft0.z - cross.z);
            glVertex3f(ft1.x - cross.x, ft1.y - cross.y, ft1.z - cross.z);
            glEnd();

            // support structure
            if (add_support_structure && i % 2000 == 0) {
                Pnt3f ft2 = track_points[(i+250) % track_points.size()].pos;
                glBegin(GL_QUADS);
                if ( !doingShadows ) glColor3ub(78 , 36 , 13);
                glVertex3f(ft0.x + cross.x, ft0.y + cross.y, ft0.z + cross.z);
                glVertex3f(ft2.x + cross.x, ft2.y + cross.y, ft2.z + cross.z);
                glVertex3f(ft2.x + cross.x, 0, ft2.z + cross.z);
                glVertex3f(ft0.x + cross.x, 0, ft0.z + cross.z);

                glVertex3f(ft0.x - cross.x, ft0.y - cross.y, ft0.z - cross.z);
                glVertex3f(ft2.x - cross.x, ft2.y - cross.y, ft2.z - cross.z);
                glVertex3f(ft2.x - cross.x, 0, ft2.z - cross.z);
                glVertex3f(ft0.x - cross.x, 0, ft0.z - cross.z);
                glEnd();
            }
        }
        break;
    case 2: // road track
        for (int i=0; i < track_points.size(); ++i) {
            Pnt3f
                    ft0 = track_points[i].pos,
                    ft1 = track_points[(i+1) % track_points.size()].pos,
                    ot0 = track_points[i].orient,
                    cross;
            // cross
            cross = (ft1 - ft0) * ot0;
            cross.normalize();
            cross = cross * 2.5f;

            glBegin(GL_QUADS);
            if ( !doingShadows ) glColor3ub(32 , 32 , 64);
            glVertex3f(ft0.x + cross.x, ft0.y + cross.y, ft0.z + cross.z);
            glVertex3f(ft1.x + cross.x, ft1.y + cross.y, ft1.z + cross.z);
            glVertex3f(ft1.x - cross.x, ft1.y - cross.y, ft1.z - cross.z);
            glVertex3f(ft0.x - cross.x, ft0.y - cross.y, ft0.z - cross.z);
            glEnd();

            // support structure
            if (add_support_structure && i % 2000 == 0) {
                Pnt3f ft2 = track_points[(i+250) % track_points.size()].pos;
                glBegin(GL_QUADS);
                if ( !doingShadows ) glColor3ub(78 , 36 , 13);
                glVertex3f(ft0.x + cross.x, ft0.y + cross.y, ft0.z + cross.z);
                glVertex3f(ft2.x + cross.x, ft2.y + cross.y, ft2.z + cross.z);
                glVertex3f(ft2.x + cross.x, 0, ft2.z + cross.z);
                glVertex3f(ft0.x + cross.x, 0, ft0.z + cross.z);

                glVertex3f(ft0.x - cross.x, ft0.y - cross.y, ft0.z - cross.z);
                glVertex3f(ft2.x - cross.x, ft2.y - cross.y, ft2.z - cross.z);
                glVertex3f(ft2.x - cross.x, 0, ft2.z - cross.z);
                glVertex3f(ft0.x - cross.x, 0, ft0.z - cross.z);
                glEnd();
            }
        }
        break;
    default:
        break;
    }

    glPopMatrix();
    glDisable(GL_DEPTH_TEST);
//    glDisable(GL_LIGHTING);
//    glDisable(GL_LIGHT0);
//    glDisable(GL_LIGHT1);
}

/*
 * compute and save track points
 * linear curve, cardinal curve, cubic curve
 */
void OpenGLView::computeTrack()
{
    track_points.clear();
    switch (curve) {
    case 0: // linear curve
        for (int ti=0; ti < m_pTrack.size(); ++ti) {
            for (uint i=0; i < m_pTrack[ti].points.size(); ++i) {
                // pos
                Pnt3f
                        p1_pos = m_pTrack[ti].points[i].pos,
                        p2_pos = m_pTrack[ti].
                        points[(i + 1) % m_pTrack[ti].points.size()].pos,
                // orient
                        p1_orient = m_pTrack[ti].points[i].orient,
                        p2_orient = m_pTrack[ti].
                        points[(i + 1) % m_pTrack[ti].points.size()].orient;

                // small segment, precompute STD_DIVIDE_LINE / 50
                float
                        t = 0,
                        percent = 50.0f / ((p1_pos - p2_pos).length() * STD_DIVIDE_LINE);

                while(t < 1) {
                    Pnt3f
                            ft =  p1_pos + t * (p2_pos - p1_pos),
                            orient_t = p1_orient + t * (p2_orient - p1_orient);

                    orient_t.normalize();
                    track_points.push(Pnt3_data(ft, orient_t));

                    t += percent; // next
                }
            }
        }
        break;
    case 1: // cardinal curve
    {
        const HMatrix M = {
            -.5f, 1, -.5f, 0,
            1.5f, -2.5f, 0, 1,
            -1.5f, 2, .5f, 0,
            .5f, -.5f, 0, 0
        };
        computeSpline(M);
        break;
    }
    case 2: // cubic curve
    {
        const HMatrix M = {
            -0.16666666666f, .5f, -.5f, 0.16666666666f,
            .5f, -1, 0, 0.66666666666f,
            -.5f, .5f, .5f, 0.16666666666f,
            0.16666666666f, 0, 0, 0
        };
        computeSpline(M);
        break;
    }
    default:
        break;
    }
}

/*
 * cardinal curve, cubic curve
 */
void OpenGLView::computeSpline(const HMatrix M)
{
    for (int ti=0; ti < m_pTrack.size(); ++ti) {
        int size = (int)m_pTrack[ti].points.size();
        for (int i=size-1; i < size * 2 - 1; ++i) {
            Pnt3f
                    p0_orient = m_pTrack[ti].points[i % size].orient,

                    p1_orient = m_pTrack[ti].
                    points[(i + 1) % size].orient,

                    p2_orient = m_pTrack[ti].
                    points[(i + 2) % size].orient,

                    p3_orient = m_pTrack[ti].
                    points[(i + 3) % size].orient,

                    G0 = m_pTrack[ti].points[i % size].pos,

                    G1 = m_pTrack[ti].
                    points[(i + 1) % size].pos,

                    G2 = m_pTrack[ti].
                    points[(i + 2) % size].pos,

                    G3 =m_pTrack[ti].
                    points[(i + 3) % size].pos;

            // small segment, C = GM
            float
                    t = 0,
                    G[3][4] = {
                G0.x, G1.x, G2.x, G3.x,
                G0.y, G1.y, G2.y, G3.y,
                G0.z, G1.z, G2.z, G3.z,
            },
                    C[3][4] = {
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0
            };
            for (int r=0; r < 3; ++r)
                for (int c=0; c < 4; ++c)
                    for (int j=0; j < 4; ++j)
                        C[r][c] += G[r][j] * M[j][c];

            float percent = 1.0f / STD_DIVIDE_LINE, approximate_dist = 0;
            // compute STD_DIVIDE_LINE of points' total length
            while(t < 1) {
                Pnt3f ft0, ft1;
                ft0.x =
                        C[0][0] * t * t * t +
                        C[0][1] * t * t +
                        C[0][2] * t +
                        C[0][3];
                ft0.y =
                        C[1][0] * t * t * t +
                        C[1][1] * t * t +
                        C[1][2] * t +
                        C[1][3];
                ft0.z =
                        C[2][0] * t * t * t +
                        C[2][1] * t * t +
                        C[2][2] * t +
                        C[2][3];

                t += percent; // next

                ft1.x =
                        C[0][0] * t * t * t +
                        C[0][1] * t * t +
                        C[0][2] * t +
                        C[0][3];
                ft1.y =
                        C[1][0] * t * t * t +
                        C[1][1] * t * t +
                        C[1][2] * t +
                        C[1][3];
                ft1.z =
                        C[2][0] * t * t * t +
                        C[2][1] * t * t +
                        C[2][2] * t +
                        C[2][3];

                approximate_dist += (ft0 - ft1).length();
            }

            float
                    orient_g[3][4] = {
                p0_orient.x, p1_orient.x, p2_orient.x, p3_orient.x,
                p0_orient.y, p1_orient.y, p2_orient.y, p3_orient.y,
                p0_orient.z, p1_orient.z, p2_orient.z, p3_orient.z
            },
                    orient_c[3][4] = {
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0
            };
            for (int r=0; r < 3; ++r)
                for (int c=0; c < 4; ++c)
                    for (int j=0; j < 4; ++j)
                        orient_c[r][c] += orient_g[r][j] * M[j][c];


            // precompute STD_DIVIDE_LINE / 50
            percent = 50.0f / (approximate_dist * STD_DIVIDE_LINE);
            t = 0;
            while(t < 1) {
                Pnt3f ft, orient_t;

                ft.x =
                        C[0][0] * t * t * t +
                        C[0][1] * t * t +
                        C[0][2] * t +
                        C[0][3];
                ft.y =
                        C[1][0] * t * t * t +
                        C[1][1] * t * t +
                        C[1][2] * t +
                        C[1][3];
                ft.z =
                        C[2][0] * t * t * t +
                        C[2][1] * t * t +
                        C[2][2] * t +
                        C[2][3];

                orient_t.x =
                        orient_c[0][0] * t * t * t +
                        orient_c[0][1] * t * t +
                        orient_c[0][2] * t +
                        orient_c[0][3];
                orient_t.y =
                        orient_c[1][0] * t * t * t +
                        orient_c[1][1] * t * t +
                        orient_c[1][2] * t +
                        orient_c[1][3];
                orient_t.z =
                        orient_c[2][0] * t * t * t +
                        orient_c[2][1] * t * t +
                        orient_c[2][2] * t +
                        orient_c[2][3];

                orient_t.normalize();
                track_points.push(Pnt3_data(ft, orient_t));

                t += percent; // next
            }
        }
    }
}

/*************************************************************************
 * draw train that run on the track
 * track_points' orient means up vector not tangent vector
 * use histogram like CDF to let the train's velocity be same
 *************************************************************************/
/// TODO:
/// histogram
void OpenGLView::runTrain()
{
    float last_t_time = t_time;
    for (int i=0; i < trains_index.size(); ++i) {
        int
                obj_index = trains_index[i],
                track_index = last_t_time * track_points.size();
        Pnt3_data
                p = track_points[track_index],
                p2 = track_points[(track_index + 1) % track_points.size()];

        Pnt3f forward = p2.pos - p.pos;
        forward.normalize();

        objs[obj_index]->pos.x = p.pos.x + p.orient.x * 5;
        objs[obj_index]->pos.y = p.pos.y + p.orient.y * 5;
        objs[obj_index]->pos.z = p.pos.z + p.orient.z * 5;

        objs[obj_index]->use_rotation_matrix = true;
        Pnt3f
                new_x = forward * p.orient,
                new_y = new_x * forward;
        new_x.normalize();
        new_y.normalize();

        if (camera == 2 && i == 0) {
            train_view_eye = p.pos + new_y * 15;
            train_view_center = p.pos + forward * 50 + new_y * 15;
            train_view_up = new_y;
        }

        float m[16] = {
            new_x.x, new_y.x, forward.x, 0,
            new_x.y, new_y.y, forward.y, 0,
            new_x.z, new_y.z, forward.z, 0,
            0, 0, 0, 1
        };
        for (int j=0; j < 16; ++j)
            objs[obj_index]->rotation_matrix[j / 4][j % 4] = m[j];

        last_t_time -= (train_speed + 500) / 10e3;
        if (last_t_time >= 1) last_t_time = fmod(last_t_time, 1);
        else if (last_t_time < 0) last_t_time = fmod(last_t_time, 1) + 1;
    }
    t_time += train_speed / 10e3;
    if (t_time >= 1) t_time = fmod(t_time, 1);
    else if (t_time < 0) t_time = fmod(t_time, 1) + 1;
}
