QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = P3
TEMPLATE = app

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    OpenGLView.cpp \
    OQ3DUtilities.cpp \
    ArcBallCam.CPP \
    Track.CPP \
    ControlPoint.CPP \
    Pnt3f.CPP \
    Model.CPP

HEADERS += \
    mainwindow.h \
    OpenGLView.h \
    OQ3DUtilities.h \
    ArcBallCam.H \
    Track.H \
    ControlPoint.H \
    Pnt3f.H \
    Model.H \
    Point3d.H

FORMS += mainwindow.UI

RESOURCES += \
    resource.qrc
