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
    void on_action_hide_whole_track_triggered();
    void on_action_hide_control_points_triggered();

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
    void on_action_load_obj_triggered();

    void on_x_doubleSpinBox_valueChanged(double arg1);
    void on_y_doubleSpinBox_valueChanged(double arg1);
    void on_z_doubleSpinBox_valueChanged(double arg1);

    void on_translate_radioButton_toggled(bool checked);
    void on_extend_radioButton_toggled(bool checked);
    void on_rotate_radioButton_toggled(bool checked);

    void on_do_x_checkBox_toggled(bool checked);
    void on_do_y_checkBox_toggled(bool checked);
    void on_do_z_checkBox_toggled(bool checked);

    void on_delete_obj_pushButton_clicked();

    void on_take_away_pushButton_clicked();
    void on_put_on_track_pushButton_clicked();

    void on_train_speed_slider_valueChanged(int value);

    void on_action_save_scene_triggered();
    void on_action_load_scence_triggered();

    void on_action_add_support_structure_triggered();

    void on_action_add_new_track_triggered();
    void on_action_remove_last_track_triggered();

    void on_obj_listWidget_itemPressed(QListWidgetItem *item);

    void on_action_shader_default_triggered();
    void on_action_shader_negative_triggered();
    void on_action_shader_blur_triggered();
    void on_action_shader_hallucination_triggered();
    void on_action_load_fragment_shader_triggered();

    void on_light_type_comboBox_currentIndexChanged(int index);

    void on_spot_light_angle_doubleSpinBox_valueChanged(double arg1);
    void on_light_power_doubleSpinBox_valueChanged(double arg1);

    void on_light_color_radioButton_toggled(bool checked);
    void on_light_orientation_radioButton_toggled(bool checked);

    void on_action_use_fbo_triggered(bool checked);

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
