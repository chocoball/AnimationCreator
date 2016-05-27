#-------------------------------------------------
#
# Project created by QtCreator 2010-11-04T21:23:30
#
#-------------------------------------------------

QT += core gui opengl xml network help

CONFIG += console
QMAKE_CXXFLAGS += -std=c++11

TARGET = AnimationCreator2
TEMPLATE = app


SOURCES += \
    src/animationform.cpp \
    src/animationwindowsplitter.cpp \
    src/data/anm2dbase.cpp \
    src/data/anm2dbin.cpp \
    src/data/anm2dxml.cpp \
    src/data/anm2djson.cpp \
    src/data/anm2dasm.cpp \
    src/application.cpp \
    src/brightlabel.cpp \
    src/datamarkerlabel.cpp \
    src/dropablemdiarea.cpp \
    src/loupewindow.cpp \
    src/colorpickerform.cpp \
    src/command.cpp \
    src/curveeditorform.cpp \
    src/curvegraphlabel.cpp \
    src/debug.cpp \
    src/editdata.cpp \
    src/exportpngform.cpp \
    src/framedatascaleform.cpp \
    src/glwidget.cpp \
    src/gridlabel.cpp \
    src/helpbrowser.cpp \
    src/helpwindow.cpp \
    src/imagewindow.cpp \
    src/keyboardmodel.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/objectitem.cpp \
    src/objectmodel.cpp \
    src/objectscaleform.cpp \
    src/optiondialog.cpp \
    src/setting.cpp \
    src/texturecachemanager.cpp \
    src/util.cpp

HEADERS += \
    src/animationform.h \
    src/animationwindowsplitter.h \
    src/data/anm2dtypes.h \
    src/data/anm2d.h \
    src/data/anm2dbase.h \
    src/data/anm2dbin.h \
    src/data/anm2dxml.h \
    src/data/anm2djson.h \
    src/data/anm2dasm.h \
    src/application.h \
    src/brightlabel.h \
    src/datamarkerlabel.h \
    src/dropablemdiarea.h \
    src/loupewindow.h \
    src/colorpickerform.h \
    src/command.h \
    src/rect.h \
    src/curveeditorform.h \
    src/curvegraphlabel.h \
    src/debug.h \
    src/defines.h \
    src/editdata.h \
    src/exportpngform.h \
    src/framedata.h \
    src/framedatascaleform.h \
    src/glwidget.h \
    src/gridlabel.h \
    src/helpbrowser.h \
    src/helpwindow.h \
    src/imagewindow.h \
    src/include.h \
    src/keyboardmodel.h \
    src/mainwindow.h \
    src/objectitem.h \
    src/objectmodel.h \
    src/objectscaleform.h \
    src/optiondialog.h \
    src/setting.h \
    src/texturecachemanager.h \
    src/util.h

RESOURCES += Resource.qrc

FORMS += \
    ui/animationform.ui \
    ui/colorpickerform.ui \
    ui/curveeditorform.ui \
    ui/exportpngform.ui \
    ui/framedatascaleform.ui \
    ui/imagewindow.ui \
    ui/keyboardtab.ui \
    ui/objectscaleform.ui \
    ui/optionanimationtab.ui \
    ui/optionfiletab.ui
















