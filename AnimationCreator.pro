#-------------------------------------------------
#
# Project created by QtCreator 2010-11-04T21:23:30
#
#-------------------------------------------------

QT       += core gui opengl xml

TARGET = AnimationCreator
TEMPLATE = app


SOURCES		+=								\
				src/animationform.cpp		\
				src/canm2d.cpp				\
				src/cdatamarkerlabel.cpp	\
				src/cdropablemdiarea.cpp	\
				src/cloupewindow.cpp		\
				src/command.cpp				\
				src/editdata.cpp			\
				src/glwidget.cpp			\
				src/gridlabel.cpp			\
				src/helpbrowser.cpp			\
				src/helpwindow.cpp			\
				src/imagewindow.cpp			\
				src/mainwindow.cpp			\
				src/main.cpp				\
				src/objectmodel.cpp			\
				src/optiondialog.cpp		\
				src/exportpngform.cpp		\
				src/util.cpp				\
				src/colorpickerform.cpp		\
				src/CBrightLabel.cpp

HEADERS		+=								\
				src/Anm2dTypes.h			\
				src/animationform.h			\
				src/canm2d.h				\
				src/cdatamarkerlabel.h		\
				src/cdropablemdiarea.h		\
				src/cloupewindow.h			\
				src/command.h				\
				src/defines.h				\
				src/editdata.h				\
				src/glwidget.h				\
				src/gridlabel.h				\
				src/helpbrowser.h			\
				src/helpwindow.h			\
				src/imagewindow.h			\
				src/mainwindow.h			\
				src/objectmodel.h			\
				src/optiondialog.h			\
				src/setting.h				\
				src/exportpngform.h			\
				src/util.h					\
				src/colorpickerform.h		\
				src/CBrightLabel.h

RESOURCES	+=	Resource.qrc

FORMS		+= 	ui/animationform.ui			\
				ui/imagewindow.ui			\
				ui/exportpngform.ui			\
				ui/OptionAnimationTab.ui	\
				ui/colorpickerform.ui

CONFIG		+=	help
