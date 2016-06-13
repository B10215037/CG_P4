#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMouseEvent>
#include <QtOpenGL/qgl.h>
#include "OpenGLView.h"
#include "OQ3DUtilities.h"
#include "Track.H"
#include <time.h>
#include <math.h>

//class CTrack;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    // call this method when things change
    void damageMe();

    void ToggleMenuBar();
    void ToggleToolBar();
    void ToggleStatusBar();

    // the widgets that make up the Window
    OpenGLView*		opengl_view;

    // keep track of the stuff in the world
//    QStack<CTrack>	m_Track;

    bool canpan = false;
    bool isHover = false;

protected:
    bool eventFilter(QObject *watched, QEvent *e);

private slots:
    void LoadTrackPath();
    void SaveTrackPath();
    void ExitApp();

    void ChangeCameraType( QString type );
    void ChangeCamToWorld();
    void ChangeCamToTop();
    void ChangeCamToTrain();

    void ChangeCurveType( QString type );
    void ChangeCurveToLinear();
    void ChangeCurveToCardinal();
    void ChangeCurveToCubic();

    void ChangeTrackType( QString type );
    void ChangeTrackToLine();
    void ChangeTrackToTrack();
    void ChangeTrackToRoad();
    void on_actionHide_all_triggered();
    void on_actionHide_control_points_triggered();

    void SwitchPlayAndPause();

    void AddControlPoint();
    void DeleteControlPoint();

    void RotateControlPointAddX();
    void RotateControlPointSubX();
    void RotateControlPointAddZ();
    void RotateControlPointSubZ();

    /*************************************************************************
     * new function add
     *************************************************************************/
    void on_actionLoad_Obj_triggered();

    void on_x_doubleSpinBox_valueChanged(double arg1);
    void on_y_doubleSpinBox_valueChanged(double arg1);
    void on_z_doubleSpinBox_valueChanged(double arg1);

    void on_move_radioButton_toggled(bool checked);
    void on_scale_radioButton_toggled(bool checked);
    void on_rotate_radioButton_toggled(bool checked);

    void on_do_x_checkBox_toggled(bool checked);
    void on_do_y_checkBox_toggled(bool checked);
    void on_do_z_checkBox_toggled(bool checked);

    void on_delete_obj_pushButton_clicked();

    void on_put_on_track_pushButton_clicked();
    void on_take_away_pushButton_clicked();

    void on_sSpeed_valueChanged(int value);

    void on_actionSave_Scene_triggered();

    void on_actionLoad_Scence_triggered();

    void on_light_comboBox_currentIndexChanged(int index);
    void on_light_attribute_comboBox_currentIndexChanged(int index);

    void on_light_x_doubleSpinBox_valueChanged(double arg1);
    void on_light_y_doubleSpinBox_valueChanged(double arg1);
    void on_light_z_doubleSpinBox_valueChanged(double arg1);

    void on_actionAdd_support_structure_triggered();

    void on_actionAdd_new_track_triggered();
    void on_actionRemove_last_track_triggered();

    void on_listWidget_itemPressed(QListWidgetItem *item);

    void on_actionShader_default_triggered();
    void on_actionShader_negative_triggered();
    void on_actionShader_blur_triggered();
    void on_actionShader_hallucination_triggered();

    void on_actionLoad_fragment_shader_triggered();

private:
    void UpdateCameraState( int index );
    void UpdateCurveState( int index );
    void UpdateTrackState( int index );
    void UpdateVelocityState( int index ); ///TODO
    void rollx( float dir );
    void rollz( float dir );

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
