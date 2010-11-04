#-------------------------------------------------
#
# Project created by QtCreator 2010-11-04T21:23:30
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = AnimationCreator
TEMPLATE = app


SOURCES +=	src/objectmodel.cpp			\
			src/mainwindow.cpp			\
			src/main.cpp				\
			src/imagewindow.cpp			\
			src/gridlabel.cpp			\
			src/glwidget.cpp			\
			src/editimagedata.cpp		\
			src/command.cpp				\
			src/cloupewindow.cpp		\
			src/cdropablemdiarea.cpp	\
			src/cdatamarkerlabel.cpp	\
			src/canm2d.cpp				\
			src/animationform.cpp

HEADERS  += src/setting.h				\
			src/objectmodel.h			\
			src/mainwindow.h			\
			src/imagewindow.h			\
			src/gridlabel.h				\
			src/glwidget.h				\
			src/editimagedata.h			\
			src/command.h				\
			src/cloupewindow.h			\
			src/cdropablemdiarea.h		\
			src/cdatamarkerlabel.h		\
			src/canm2d.h				\
			src/Anm2dTypes.h			\
			src/animationform.h

RESOURCES += \
			Resource.qrc

FORMS += \
			ui/animationform.ui
