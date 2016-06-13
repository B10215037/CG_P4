#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    opengl_view = new OpenGLView();
//    opengl_view->m_pTrack = m_Track;
    setGeometry(200, 30, 1000, 500);
    ui->mainLayout->layout()->addWidget(opengl_view);
    opengl_view->installEventFilter(this);
    canpan = false;
    isHover = false;
    opengl_view->camera = 0;
    opengl_view->track = 0;
    opengl_view->curve = 0;
    opengl_view->isrun = false;

    setWindowTitle( "Roller Coaster" );

    //signals & slots
    connect( ui->aLoadPath      ,SIGNAL(triggered()),this,SLOT(LoadTrackPath()) );
    connect( ui->aSavePath      ,SIGNAL(triggered()),this,SLOT(SaveTrackPath())	);
    connect( ui->aExit          ,SIGNAL(triggered()),this,SLOT(ExitApp())       );

    connect( ui->aWorld         ,SIGNAL(triggered()),this,SLOT(ChangeCamToWorld())  );
    connect( ui->aTop           ,SIGNAL(triggered()),this,SLOT(ChangeCamToTop())    );
    connect( ui->aTrain         ,SIGNAL(triggered()),this,SLOT(ChangeCamToTrain())  );

    connect( ui->aLinear		,SIGNAL(triggered()),this,SLOT(ChangeCurveToLinear())   );
    connect( ui->aCardinal      ,SIGNAL(triggered()),this,SLOT(ChangeCurveToCardinal()) );
    connect( ui->aCubic         ,SIGNAL(triggered()),this,SLOT(ChangeCurveToCubic())    );

    connect( ui->aLine          ,SIGNAL(triggered()),this,SLOT(ChangeTrackToLine())     );
    connect( ui->aTrack         ,SIGNAL(triggered()),this,SLOT(ChangeTrackToTrack())    );
    connect( ui->aRoad          ,SIGNAL(triggered()),this,SLOT(ChangeTrackToRoad())		);

    connect( ui->bPlay          ,SIGNAL(clicked()),this,SLOT(SwitchPlayAndPause())		);
    connect( ui->bAdd           ,SIGNAL(clicked()),this,SLOT(AddControlPoint())			);
    connect( ui->bDelete        ,SIGNAL(clicked()),this,SLOT(DeleteControlPoint())		);

    connect( ui->rcpxadd		,SIGNAL(clicked()),this,SLOT(RotateControlPointAddX())  );
    connect( ui->rcpxsub		,SIGNAL(clicked()),this,SLOT(RotateControlPointSubX())	);
    connect( ui->rcpzadd		,SIGNAL(clicked()),this,SLOT(RotateControlPointAddZ())  );
    connect( ui->rcpzsub		,SIGNAL(clicked()),this,SLOT(RotateControlPointSubZ())  );

    ui->move_radioButton->setChecked(true);
    ui->scale_radioButton->setChecked(false);
    ui->rotate_radioButton->setChecked(false);
    ui->do_x_checkBox->setChecked(true);
    ui->do_y_checkBox->setChecked(true);
    ui->do_z_checkBox->setChecked(true);

    ui->light_x_doubleSpinBox->setValue(Model::light1.position.x());
    ui->light_y_doubleSpinBox->setValue(Model::light1.position.y());
    ui->light_z_doubleSpinBox->setValue(Model::light1.position.z());
}

void MainWindow::damageMe()
{
    if (opengl_view->selectedCube[0] >= opengl_view->m_pTrack.size() ||
            opengl_view->selectedCube[1] >=
            ((int)opengl_view->m_pTrack[opengl_view->selectedCube[0]].
             points.size())) {
        opengl_view->selectedCube[0] = 0;
        opengl_view->selectedCube[1] = 0;
    }
}

void MainWindow::ToggleMenuBar()
{
    ui->menuBar->setHidden( !ui->menuBar->isHidden() );
}

void MainWindow::ToggleToolBar()
{
    ui->mainToolBar->setHidden( !ui->mainToolBar->isHidden() );
}

void MainWindow::ToggleStatusBar()
{
    ui->statusBar->setHidden( !ui->statusBar->isHidden() );
}

bool MainWindow::eventFilter(QObject *watched, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *event = static_cast<QMouseEvent*> (e);
        // Get the mouse position
        float x, y;
        opengl_view->arcball.getMouseNDC((float)(event->localPos().x()),
                                         (float)event->localPos().y(), x,y);

        // Compute the mouse position
        opengl_view->arcball.down(x, y);
        if (event->button() == Qt::LeftButton) {
            opengl_view->doPick(event->localPos().x(), event->localPos().y());
            if (canpan)
                opengl_view->arcball.mode = opengl_view->arcball.Pan;
            else
                isHover = true;

            // after pick obj, show some attribute to user
            if (opengl_view->selected_obj >= 0) {
                Model *obj = opengl_view->objs[opengl_view->selected_obj];
                if (ui->move_radioButton->isChecked()) {
                    ui->x_doubleSpinBox->setValue(obj->pos.x);
                    ui->y_doubleSpinBox->setValue(obj->pos.y);
                    ui->z_doubleSpinBox->setValue(obj->pos.z);
                }
                else if (ui->scale_radioButton->isChecked()) {
                    ui->x_doubleSpinBox->setValue(obj->scale.x);
                    ui->y_doubleSpinBox->setValue(obj->scale.y);
                    ui->z_doubleSpinBox->setValue(obj->scale.z);
                }
                else if (ui->rotate_radioButton->isChecked()) {
                    ui->x_doubleSpinBox->setValue(obj->rotation.x);
                    ui->y_doubleSpinBox->setValue(obj->rotation.y);
                    ui->z_doubleSpinBox->setValue(obj->rotation.z);
                }
            }
        }
        if (event->button() == Qt::RightButton) {
            opengl_view->arcball.mode = opengl_view->arcball.Rotate;
        }
    }

    if (e->type() == QEvent::MouseButtonRelease) {
        isHover = false;
        opengl_view->arcball.mode = opengl_view->arcball.None;
    }

    if (e->type() == QEvent::Wheel) {
        QWheelEvent *event = static_cast<QWheelEvent*> (e);
        float zamt = (event->delta() < 0) ? 1.1f : 1/1.1f;
        opengl_view->arcball.eyeZ *= zamt;
    }

    if (e->type() == QEvent::MouseMove) {
        QMouseEvent *event = static_cast<QMouseEvent*> (e);
        if (isHover && opengl_view->selectedCube[0] >= 0) {
            ControlPoint* cp =
                    &opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                    points[opengl_view->selectedCube[1]];

            double r1x, r1y, r1z, r2x, r2y, r2z;
            int x = event->localPos().x();
            int iy = event->localPos().y();
            // we have to deal with the projection matrices
            double mat1[16], mat2[16];
            int viewport[4];

            glGetIntegerv(GL_VIEWPORT, viewport);
            glGetDoublev(GL_MODELVIEW_MATRIX, mat1);
            glGetDoublev(GL_PROJECTION_MATRIX, mat2);

            int y = viewport[3] - iy; // originally had an extra -1?

            gluUnProject((double) x, (double) y, .25,
                         mat1, mat2, viewport, &r1x, &r1y, &r1z);
            gluUnProject((double) x, (double) y, 0,
                          mat1, mat2, viewport, &r2x, &r2y, &r2z);

            double rx, ry, rz;
            mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
                static_cast<double>(cp->pos.x),
                static_cast<double>(cp->pos.y),
                static_cast<double>(cp->pos.z),
                rx, ry, rz,
                false);

            cp->pos.x = (float) rx;
            cp->pos.y = (float) ry;
            cp->pos.z = (float) rz;

            opengl_view->computeTrack(); // recompute
        }
        else if (isHover && opengl_view->selected_obj >= 0) {
            Model *obj = opengl_view->objs[opengl_view->selected_obj];

            double r1x, r1y, r1z, r2x, r2y, r2z;
            int x = event->localPos().x();
            int iy = event->localPos().y();
            // we have to deal with the projection matrices
            double mat1[16], mat2[16];
            int viewport[4];

            glGetIntegerv(GL_VIEWPORT, viewport);
            glGetDoublev(GL_MODELVIEW_MATRIX, mat1);
            glGetDoublev(GL_PROJECTION_MATRIX, mat2);

            int y = viewport[3] - iy; // originally had an extra -1?

            gluUnProject((double) x, (double) y, .25,
                         mat1, mat2, viewport, &r1x, &r1y, &r1z);
            gluUnProject((double) x, (double) y, 0,
                          mat1, mat2, viewport, &r2x, &r2y, &r2z);

            double rx, ry, rz;
            mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
                static_cast<double>(obj->pos.x),
                static_cast<double>(obj->pos.y),
                static_cast<double>(obj->pos.z),
                rx, ry, rz,
                false);

            if (ui->move_radioButton->isChecked()) {
                if (ui->do_x_checkBox->isChecked())
                    ui->x_doubleSpinBox->setValue(rx);
                if (ui->do_y_checkBox->isChecked())
                    ui->y_doubleSpinBox->setValue(ry);
                if (ui->do_z_checkBox->isChecked())
                    ui->z_doubleSpinBox->setValue(rz);
            }
        }
        // we're taking the drags
        if (opengl_view->arcball.mode != opengl_view->arcball.None) {
            float x,y;
            opengl_view->arcball.getMouseNDC((float)event->localPos().x(),
                                           (float)event->localPos().y(),x,y);
            opengl_view->arcball.computeNow(x,y);
        }
    }

    if (e->type() == QEvent::KeyPress) {
         QKeyEvent *event = static_cast< QKeyEvent*> (e);
        // Set up the mode
        if (event->key() == Qt::Key_Alt)
            canpan = true;
        else if (event->key() == Qt::Key_X)
            ui->do_x_checkBox->setChecked(!ui->do_x_checkBox->isChecked());
        else if (event->key() == Qt::Key_Y)
            ui->do_y_checkBox->setChecked(!ui->do_y_checkBox->isChecked());
        else if (event->key() == Qt::Key_Z)
            ui->do_z_checkBox->setChecked(!ui->do_z_checkBox->isChecked());
        else if (event->key() == Qt::Key_T) // translate
            ui->move_radioButton->setChecked(!ui->move_radioButton->isChecked());
        else if (event->key() == Qt::Key_E) // extend
            ui->scale_radioButton->setChecked(!ui->scale_radioButton->isChecked());
        else if (event->key() == Qt::Key_R) // rotate
            ui->rotate_radioButton->setChecked(!ui->rotate_radioButton->isChecked());
        else if (event->key() == Qt::Key_W) // move camera forward
            opengl_view->arcball.eyeZ -= 2;
        else if (event->key() == Qt::Key_A) // move camera left
            opengl_view->arcball.eyeX -= 2;
        else if (event->key() == Qt::Key_S) // move camera backward
            opengl_view->arcball.eyeZ += 2;
        else if (event->key() == Qt::Key_D) // move camera right
            opengl_view->arcball.eyeX += 2;
        else if (event->key() == Qt::Key_I) // move camera upward
            opengl_view->arcball.eyeY += 2;
        else if (event->key() == Qt::Key_K) // move camera downward
            opengl_view->arcball.eyeY -= 2;
        else if (event->key() == Qt::Key_Up) { // rotate camera upward
            opengl_view->arcball.down(0, 0);
            opengl_view->arcball.now.x -= .02f;
        }
        else if (event->key() == Qt::Key_Down) { // rotate camera downward
            opengl_view->arcball.down(0, 0);
            opengl_view->arcball.now.x += .02f;
        }
        else if (event->key() == Qt::Key_Left) { // rotate camera leftward
            opengl_view->arcball.down(0, 0);
            opengl_view->arcball.now.y -= .02f;
        }
        else if (event->key() == Qt::Key_Right) { // rotate camera rightward
            opengl_view->arcball.down(0, 0);
            opengl_view->arcball.now.y += .02f;
        }
        else if (event->key() == Qt::Key_PageUp) { // add
            if (opengl_view->selectedCube[0] >= 0) {
                opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                        points[opengl_view->selectedCube[1]].pos.y++;
                opengl_view->computeTrack();
            }
            else if (opengl_view->selected_obj >= 0) {
                Model *obj = opengl_view->objs[opengl_view->selected_obj];
                if (ui->move_radioButton->isChecked()) {
                    if (ui->do_x_checkBox->isChecked())
                        ui->x_doubleSpinBox->setValue(++obj->pos.x);
                    if (ui->do_y_checkBox->isChecked())
                        ui->y_doubleSpinBox->setValue(++obj->pos.y);
                    if (ui->do_z_checkBox->isChecked())
                        ui->z_doubleSpinBox->setValue(++obj->pos.z);
                }
                else if (ui->scale_radioButton->isChecked()) {
                    if (ui->do_x_checkBox->isChecked())
                        ui->x_doubleSpinBox->setValue(++obj->scale.x);
                    if (ui->do_y_checkBox->isChecked())
                        ui->y_doubleSpinBox->setValue(++obj->scale.y);
                    if (ui->do_z_checkBox->isChecked())
                        ui->z_doubleSpinBox->setValue(++obj->scale.z);
                }
                else if (ui->rotate_radioButton->isChecked()) {
                    if (ui->do_x_checkBox->isChecked())
                        ui->x_doubleSpinBox->setValue(++obj->rotation.x);
                    if (ui->do_y_checkBox->isChecked())
                        ui->y_doubleSpinBox->setValue(++obj->rotation.y);
                    if (ui->do_z_checkBox->isChecked())
                        ui->z_doubleSpinBox->setValue(++obj->rotation.z);
                }
            }
        }
        else if (event->key() == Qt::Key_PageDown) { // sub
            if (opengl_view->selectedCube[0] >= 0) {
                opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                        points[opengl_view->selectedCube[1]].pos.y--;
                opengl_view->computeTrack();
            }
            else if (opengl_view->selected_obj >= 0) {
                Model *obj = opengl_view->objs[opengl_view->selected_obj];
                if (ui->move_radioButton->isChecked()) {
                    if (ui->do_x_checkBox->isChecked())
                        ui->x_doubleSpinBox->setValue(--obj->pos.x);
                    if (ui->do_y_checkBox->isChecked())
                        ui->y_doubleSpinBox->setValue(--obj->pos.y);
                    if (ui->do_z_checkBox->isChecked())
                        ui->z_doubleSpinBox->setValue(--obj->pos.z);
                }
                else if (ui->scale_radioButton->isChecked()) {
                    if (ui->do_x_checkBox->isChecked())
                        ui->x_doubleSpinBox->setValue(--obj->scale.x);
                    if (ui->do_y_checkBox->isChecked())
                        ui->y_doubleSpinBox->setValue(--obj->scale.y);
                    if (ui->do_z_checkBox->isChecked())
                        ui->z_doubleSpinBox->setValue(--obj->scale.z);
                }
                else if (ui->rotate_radioButton->isChecked()) {
                    if (ui->do_x_checkBox->isChecked())
                        ui->x_doubleSpinBox->setValue(--obj->rotation.x);
                    if (ui->do_y_checkBox->isChecked())
                        ui->y_doubleSpinBox->setValue(--obj->rotation.y);
                    if (ui->do_z_checkBox->isChecked())
                        ui->z_doubleSpinBox->setValue(--obj->rotation.z);
                }
            }
        }
    }
    if (e->type() == QEvent::KeyRelease) {
         QKeyEvent *event = static_cast< QKeyEvent*> (e);
        // Set up the mode
        if (event->key() == Qt::Key_Alt)
            canpan = false;
    }

    return QWidget::eventFilter(watched, e);
}

void MainWindow::LoadTrackPath()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Track Path",
        "./",
        tr("Txt (*.txt)" )
        );
    QByteArray byteArray = fileName.toLocal8Bit();
    const char* fname = byteArray.data();
    if ( !fileName.isEmpty() && opengl_view->selectedCube[0] >= 0) {
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].readPoints(fname);
        opengl_view->computeTrack();
    }
}

void MainWindow::SaveTrackPath()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Track Path",
        "./",
        tr("Txt (*.txt)" )
        );

    QByteArray byteArray = fileName.toLocal8Bit();
    char* fname = byteArray.data();
    if ( !fileName.isEmpty() && opengl_view->selectedCube[0] >= 0) {
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].writePoints(fname);
    }
}

void MainWindow::ExitApp()
{
    QApplication::quit();
}

void MainWindow::ChangeCameraType( QString type )
{
    if( type == "World" )
    {
        opengl_view->camera = 0;
        update();
    }
    else if( type == "Top" )
    {
        opengl_view->camera = 1;
        update();
    }
    else if( type == "Train" )
    {
        opengl_view->camera = 2;
        update();
    }
}

void MainWindow::ChangeCamToWorld()
{
    opengl_view->camera = 0;
}

void MainWindow::ChangeCamToTop()
{
    opengl_view->camera = 1;
}

void MainWindow::ChangeCamToTrain()
{
    opengl_view->camera = 2;
}

void MainWindow::ChangeCurveType( QString type )
{
    if ( type == "Linear" )
        opengl_view->curve = 0;
    else if ( type == "Cardinal" )
        opengl_view->curve = 1;
    else if ( type == "Cubic" )
        opengl_view->curve = 2;

    opengl_view->computeTrack();
}

void MainWindow::ChangeCurveToLinear()
{
    opengl_view->curve = 0;
    opengl_view->computeTrack();
}

void MainWindow::ChangeCurveToCardinal()
{
    opengl_view->curve = 1;
    opengl_view->computeTrack();
}

void MainWindow::ChangeCurveToCubic()
{
    opengl_view->curve = 2;
    opengl_view->computeTrack();
}

void MainWindow::ChangeTrackType( QString type )
{
    if ( type == "Line" )
        ChangeTrackToLine();
    else if ( type == "Track" )
        ChangeTrackToTrack();
    else if ( type == "Road" )
        ChangeTrackToRoad();
    else if ( type == "Hide all" )
        on_actionHide_all_triggered();
}

void MainWindow::ChangeTrackToLine()
{
    opengl_view->track = 0;
    if (opengl_view->selectedCube[0] >= 0)
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                hide_control_points = false;
}

void MainWindow::ChangeTrackToTrack()
{
    opengl_view->track = 1;
    if (opengl_view->selectedCube[0] >= 0)
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                hide_control_points = false;
}

void MainWindow::ChangeTrackToRoad()
{
    opengl_view->track = 2;
    if (opengl_view->selectedCube[0] >= 0)
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                hide_control_points = false;
}

void MainWindow::on_actionHide_all_triggered()
{
    opengl_view->track = 3;
    if (opengl_view->selectedCube[0] >= 0)
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                hide_control_points = true;
}

void MainWindow::on_actionHide_control_points_triggered()
{
    if (opengl_view->selectedCube[0] >= 0)
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                hide_control_points = true;
}

void MainWindow::SwitchPlayAndPause()
{
    if ( !opengl_view->isrun ) {
        ui->bPlay->setIcon(QIcon(":/Resources/Icons/play.ico"));
        opengl_view->isrun = true;
    }
    else {
        ui->bPlay->setIcon(QIcon(":/Resources/Icons/pause.ico"));
        opengl_view->isrun = false;
    }
}

void MainWindow::AddControlPoint()
{
    // get the number of points
    size_t npts =  opengl_view->m_pTrack[opengl_view->selectedCube[0]].points.size();
    // the number for the new point
    size_t newidx = (opengl_view->selectedCube[1]>=0) ? opengl_view->selectedCube[1] : 0;

    // pick a reasonable location
    size_t previdx = (newidx + npts -1) % npts;
    Pnt3f npos = ( opengl_view->m_pTrack[opengl_view->selectedCube[0]].
            points[previdx].pos +
             opengl_view->m_pTrack[opengl_view->selectedCube[0]].
            points[newidx].pos) * .5f;

     opengl_view->m_pTrack[opengl_view->selectedCube[0]].
            points.insert( opengl_view->m_pTrack[opengl_view->selectedCube[0]].
            points.begin() + newidx,npos);

    // make it so that the train doesn't move - unless its affected by this control point
    // it should stay between the same points
    if (ceil( opengl_view->m_pTrack[opengl_view->selectedCube[0]].trainU) > ((float)newidx)) {
         opengl_view->m_pTrack[opengl_view->selectedCube[0]].trainU += 1;
        if ( opengl_view->m_pTrack[opengl_view->selectedCube[0]].trainU >= npts)
             opengl_view->m_pTrack[opengl_view->selectedCube[0]].trainU -= npts;
    }
    damageMe();

    opengl_view->computeTrack();
}

void MainWindow::DeleteControlPoint()
{
    if ( opengl_view->m_pTrack[opengl_view->selectedCube[0]].points.size() > 4) {
        if (opengl_view->selectedCube[1] >= 0) {
             opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                    points.erase( opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                    points.begin() + opengl_view->selectedCube[1]);
        }
        else  opengl_view->m_pTrack[opengl_view->selectedCube[0]].points.pop_back();
    }
    damageMe();

    opengl_view->computeTrack();
}

void MainWindow::RotateControlPointAddX()
{
    rollx(1);
}

void MainWindow::RotateControlPointSubX()
{
    rollx(-1);
}

void MainWindow::RotateControlPointAddZ()
{
    rollz(1);
}

void MainWindow::RotateControlPointSubZ()
{
    rollz(-1);
}

void MainWindow::UpdateCameraState( int index )
{
    ui->aWorld->setChecked( (index==0)?true:false );
    ui->aTop->setChecked( (index==1)?true:false );
    ui->aTrain->setChecked( (index==2)?true:false );
}

void MainWindow::UpdateCurveState( int index )
{
    ui->aLinear->setChecked( (index==0)?true:false );
    ui->aCardinal->setChecked( (index==1)?true:false );
    ui->aCubic->setChecked( (index==2)?true:false );
}

void MainWindow::UpdateTrackState( int index )
{
    ui->aLine ->setChecked( (index==0)?true:false );
    ui->aTrack->setChecked( (index==1)?true:false );
    ui->aRoad ->setChecked( (index==2)?true:false );
}

/*************************************************************************
 * Rotate the selected control point about x axis
 *************************************************************************/
void MainWindow::rollx(float dir)
{
    int s = opengl_view->selectedCube[1];
    if (s >= 0) {
        Pnt3f old = opengl_view->m_pTrack[opengl_view->selectedCube[0]].points[s].orient;
        float si = sin(((float)M_PI_4) * dir);
        float co = cos(((float)M_PI_4) * dir);
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                points[s].orient.y = co * old.y - si * old.z;
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                points[s].orient.z = si * old.y + co * old.z;
    }
    damageMe();

    opengl_view->computeTrack();
}

void MainWindow::rollz(float dir)
{
    int s = opengl_view->selectedCube[1];
    if (s >= 0) {

        Pnt3f old = opengl_view->m_pTrack[opengl_view->selectedCube[0]].points[s].orient;

        float si = sin(((float)M_PI_4) * dir);
        float co = cos(((float)M_PI_4) * dir);

        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                points[s].orient.y = co * old.y - si * old.x;
        opengl_view->m_pTrack[opengl_view->selectedCube[0]].
                points[s].orient.x = si * old.y + co * old.x;
    }
    damageMe();

    opengl_view->computeTrack();
}

void MainWindow::on_actionLoad_Obj_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Obj",
        "./",
        tr("Obj (*.obj)" ));
    if ( !fileName.isEmpty() ) {
        opengl_view->objs.push(new Model(fileName));
        ui->listWidget->addItem(QFileInfo(fileName).fileName().remove(".obj"));
    }
}

void MainWindow::on_x_doubleSpinBox_valueChanged(double arg1)
{
    if (opengl_view->selected_obj >= 0 && ui->do_x_checkBox->isChecked()) {
        if (ui->move_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->pos.x = arg1;
        else if (ui->scale_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->scale.x = arg1;
        else if (ui->rotate_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->rotation.x = arg1;
    }
}

void MainWindow::on_y_doubleSpinBox_valueChanged(double arg1)
{
    if (opengl_view->selected_obj >= 0 && ui->do_y_checkBox->isChecked()) {
        if (ui->move_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->pos.y = arg1;
        else if (ui->scale_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->scale.y = arg1;
        else if (ui->rotate_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->rotation.y = arg1;
    }
}

void MainWindow::on_z_doubleSpinBox_valueChanged(double arg1)
{
    if (opengl_view->selected_obj >= 0 && ui->do_z_checkBox->isChecked()) {
        if (ui->move_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->pos.z = arg1;
        else if (ui->scale_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->scale.z = arg1;
        else if (ui->rotate_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->rotation.z = arg1;
    }
}

void MainWindow::on_move_radioButton_toggled(bool checked)
{
    if (opengl_view->selected_obj >= 0 && checked) {
        Model *obj = opengl_view->objs[opengl_view->selected_obj];
        ui->x_doubleSpinBox->setValue(obj->pos.x);
        ui->y_doubleSpinBox->setValue(obj->pos.y);
        ui->z_doubleSpinBox->setValue(obj->pos.z);
    }
}

void MainWindow::on_scale_radioButton_toggled(bool checked)
{
    if (opengl_view->selected_obj >= 0 && checked) {
        Model *obj = opengl_view->objs[opengl_view->selected_obj];
        ui->x_doubleSpinBox->setValue(obj->scale.x);
        ui->y_doubleSpinBox->setValue(obj->scale.y);
        ui->z_doubleSpinBox->setValue(obj->scale.z);
    }
}

void MainWindow::on_rotate_radioButton_toggled(bool checked)
{
    if (opengl_view->selected_obj >= 0 && checked) {
        Model *obj = opengl_view->objs[opengl_view->selected_obj];
        ui->x_doubleSpinBox->setValue(obj->rotation.x);
        ui->y_doubleSpinBox->setValue(obj->rotation.y);
        ui->z_doubleSpinBox->setValue(obj->rotation.z);
    }
}

void MainWindow::on_do_x_checkBox_toggled(bool checked)
{
    if (opengl_view->selected_obj >= 0 && checked) {
        if (ui->move_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->pos.x =
                    ui->x_doubleSpinBox->value();
        else if (ui->scale_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->scale.x =
                    ui->x_doubleSpinBox->value();
        else if (ui->rotate_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->rotation.x =
                    ui->x_doubleSpinBox->value();
    }
}

void MainWindow::on_do_y_checkBox_toggled(bool checked)
{
    if (opengl_view->selected_obj >= 0 && checked) {
        if (ui->move_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->pos.y =
                    ui->y_doubleSpinBox->value();
        else if (ui->scale_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->scale.y =
                    ui->y_doubleSpinBox->value();
        else if (ui->rotate_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->rotation.y =
                    ui->y_doubleSpinBox->value();
    }
}

void MainWindow::on_do_z_checkBox_toggled(bool checked)
{
    if (opengl_view->selected_obj >= 0 && checked) {
        if (ui->move_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->pos.z =
                    ui->z_doubleSpinBox->value();
        else if (ui->scale_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->scale.z =
                    ui->z_doubleSpinBox->value();
        else if (ui->rotate_radioButton->isChecked())
            opengl_view->objs[opengl_view->selected_obj]->rotation.z =
                    ui->z_doubleSpinBox->value();
    }
}

void MainWindow::on_delete_obj_pushButton_clicked()
{
    if (opengl_view->selected_obj >= 0) {
        delete opengl_view->objs[opengl_view->selected_obj];
        opengl_view->objs.removeAt(opengl_view->selected_obj);
        on_take_away_pushButton_clicked();
        opengl_view->selected_obj = -1;

        QModelIndexList index = ui->listWidget->selectionModel()->selectedRows();
        ui->listWidget->model()->removeRow(index[0].row());
    }
}

void MainWindow::on_take_away_pushButton_clicked()
{
    for (int i=0; i < opengl_view->trains_index.size(); ++i)
        if (opengl_view->trains_index[i] == opengl_view->selected_obj)
            opengl_view->trains_index.removeAt(i);
}

void MainWindow::on_put_on_track_pushButton_clicked()
{
    if (opengl_view->selected_obj >= 0) {
        // if already exist don't push to stack
        for (int i=0; i < opengl_view->trains_index.size(); ++i)
            if (opengl_view->trains_index[i] == opengl_view->selected_obj)
                return;

        opengl_view->trains_index.push(opengl_view->selected_obj);
    }
}

void MainWindow::on_sSpeed_valueChanged(int value)
{
    opengl_view->train_speed = value;
}

void MainWindow::on_actionSave_Scene_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Save Scene",
        "./",
        tr("Scene (*.sce)" ));
//    if ( !fileName.isEmpty() )
//        opengl_view->objs.push(Model(fileName));
}

void MainWindow::on_actionLoad_Scence_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Scence",
        "./",
        tr("Scene (*.sce)" ));
//    if ( !fileName.isEmpty() )
//        opengl_view->objs.push(Model(fileName));
}

void MainWindow::on_light_comboBox_currentIndexChanged(int index)
{
    if (index == 0) { // light 1
        if (ui->light_attribute_comboBox->currentIndex() == 0) { // position
            ui->light_x_doubleSpinBox->setValue(Model::light1.position[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light1.position[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light1.position[2]);
        }
        else { // color
            ui->light_x_doubleSpinBox->setValue(Model::light1.color[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light1.color[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light1.color[2]);
        }
    }
    else { // light 2
        if (ui->light_attribute_comboBox->currentIndex() == 0) { // position
            ui->light_x_doubleSpinBox->setValue(Model::light2.position[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light2.position[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light2.position[2]);
        }
        else { // color
            ui->light_x_doubleSpinBox->setValue(Model::light2.color[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light2.color[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light2.color[2]);
        }
    }
}

void MainWindow::on_light_attribute_comboBox_currentIndexChanged(int index)
{
    if (index == 0) { // position
        ui->light_x_label->setText("X");
        ui->light_y_label->setText("Y");
        ui->light_z_label->setText("Z");

        ui->light_x_doubleSpinBox->setMaximum(9999);
        ui->light_y_doubleSpinBox->setMaximum(9999);
        ui->light_z_doubleSpinBox->setMaximum(9999);

        ui->light_x_doubleSpinBox->setMinimum(-9999);
        ui->light_y_doubleSpinBox->setMinimum(-9999);
        ui->light_z_doubleSpinBox->setMinimum(-9999);

        ui->light_x_doubleSpinBox->setSingleStep(10);
        ui->light_y_doubleSpinBox->setSingleStep(10);
        ui->light_z_doubleSpinBox->setSingleStep(10);

        if (ui->light_comboBox->currentIndex() == 0) { // light 1
            ui->light_x_doubleSpinBox->setValue(Model::light1.position[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light1.position[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light1.position[2]);
        }
        else { // light 2
            ui->light_x_doubleSpinBox->setValue(Model::light2.position[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light2.position[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light2.position[2]);
        }
    }
    else { // color
        if (ui->light_comboBox->currentIndex() == 0) { // light 1
            ui->light_x_doubleSpinBox->setValue(Model::light1.color[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light1.color[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light1.color[2]);
        }
        else { // light 2
            ui->light_x_doubleSpinBox->setValue(Model::light2.color[0]);
            ui->light_y_doubleSpinBox->setValue(Model::light2.color[1]);
            ui->light_z_doubleSpinBox->setValue(Model::light2.color[2]);
        }

        ui->light_x_label->setText("R");
        ui->light_y_label->setText("G");
        ui->light_z_label->setText("B");

        ui->light_x_doubleSpinBox->setMaximum(1);
        ui->light_y_doubleSpinBox->setMaximum(1);
        ui->light_z_doubleSpinBox->setMaximum(1);

        ui->light_x_doubleSpinBox->setMinimum(0);
        ui->light_y_doubleSpinBox->setMinimum(0);
        ui->light_z_doubleSpinBox->setMinimum(0);

        ui->light_x_doubleSpinBox->setSingleStep(.1);
        ui->light_y_doubleSpinBox->setSingleStep(.1);
        ui->light_z_doubleSpinBox->setSingleStep(.1);
    }
}

void MainWindow::on_light_x_doubleSpinBox_valueChanged(double arg1)
{
    if (ui->light_comboBox->currentIndex() == 0) { // light 1
        if (ui->light_attribute_comboBox->currentIndex() == 0) // position
            Model::light1.position.setX(arg1);
        else // color
            Model::light1.color.setX(arg1);
    }
    else { //light 2
        if (ui->light_attribute_comboBox->currentIndex() == 0) // position
            Model::light2.position.setX(arg1);
        else // color
            Model::light2.color.setX(arg1);
    }
}

void MainWindow::on_light_y_doubleSpinBox_valueChanged(double arg1)
{
    if (ui->light_comboBox->currentIndex() == 0) { // light 1
        if (ui->light_attribute_comboBox->currentIndex() == 0) // position
            Model::light1.position.setY(arg1);
        else // color
            Model::light1.color.setY(arg1);
    }
    else { //light 2
        if (ui->light_attribute_comboBox->currentIndex() == 0) // position
            Model::light2.position.setY(arg1);
        else // color
            Model::light2.color.setY(arg1);
    }
}

void MainWindow::on_light_z_doubleSpinBox_valueChanged(double arg1)
{
    if (ui->light_comboBox->currentIndex() == 0) { // light 1
        if (ui->light_attribute_comboBox->currentIndex() == 0) // position
            Model::light1.position.setZ(arg1);
        else // color
            Model::light1.color.setZ(arg1);
    }
    else { //light 2
        if (ui->light_attribute_comboBox->currentIndex() == 0) // position
            Model::light2.position.setZ(arg1);
        else // color
            Model::light2.color.setZ(arg1);
    }
}

void MainWindow::on_actionAdd_support_structure_triggered()
{
    opengl_view->add_support_structure = !opengl_view->add_support_structure;
}

void MainWindow::on_actionAdd_new_track_triggered()
{
    opengl_view->m_pTrack.push(CTrack());
}

void MainWindow::on_actionRemove_last_track_triggered()
{
    opengl_view->m_pTrack.pop();
}

void MainWindow::on_listWidget_itemPressed(QListWidgetItem *item)
{
    opengl_view->selected_obj = ui->listWidget->row(item);
}

void MainWindow::on_actionShader_default_triggered()
{
    opengl_view->initShader("../Resources/Shaders/frame/vetex_framebuffer",
                            "../Resources/Shaders/frame/fragment_framebuffer");
}

void MainWindow::on_actionShader_negative_triggered()
{
    opengl_view->initShader("../Resources/Shaders/frame/vetex_framebuffer",
                            "../Resources/Shaders/frame/fragment_negative");
}

void MainWindow::on_actionShader_blur_triggered()
{
    opengl_view->initShader("../Resources/Shaders/frame/vetex_framebuffer",
                            "../Resources/Shaders/frame/fragment_blur_kernel");
}

void MainWindow::on_actionShader_hallucination_triggered()
{
    opengl_view->initShader("../Resources/Shaders/frame/vetex_framebuffer",
                            "../Resources/Shaders/frame/fragment_hallucination_kernel");
}

void MainWindow::on_actionLoad_fragment_shader_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Save Scene",
        "./",
        tr("txt (*)" ));
    if ( !fileName.isEmpty() )
        opengl_view->initShader("../Resources/Shaders/frame/vetex_framebuffer",
                                fileName);
}
