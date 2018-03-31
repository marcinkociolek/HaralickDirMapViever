#-------------------------------------------------
#
# Project created by QtCreator 2016-11-09T14:43:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HaralickDirMapViever
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        ../../ProjectsLib/LibMarcin/DispLib.cpp \
        ../../ProjectsLib/LibMarcin/StringFcLib.cpp \
        ../../ProjectsLib/LibMarcin/tileparams.cpp  \
        ../../ProjectsLib/LibMarcin/dirdetectionparams.cpp

HEADERS  += mainwindow.h\
         ../../ProjectsLib/LibMarcin/DispLib.h \
         ../../ProjectsLib/LibMarcin/StringFcLib.h \
         ../../ProjectsLib/LibMarcin/tileparams.h \
         ../../ProjectsLib/LibMarcin/dirdetectionparams.h

FORMS    += mainwindow.ui

win32: INCLUDEPATH += C:\opencv\build\include\
#win32: INCLUDEPATH += C:\Boost\include\boost-1_60\boost
win32: INCLUDEPATH += C:\boost_1_66_0\
win32: INCLUDEPATH += ..\..\ProjectsLib\LibMarcin\

# this is for debug
#win32: LIBS += -LC:/opencv/build/x64/vc15/lib/
#win32: LIBS += -lopencv_world341d

#win32: LIBS += -LC:/boost_1_66_0/stage/x64/lib/
#win32:  LIBS += -lboost_filesystem-vc141-mt-gd-x64-1_66
#win32:  LIBS += -lboost_regex-vc141-mt-gd-x64-1_66


# this is for release
win32: LIBS += -LC:/opencv/build/x64/vc15/lib/
win32: LIBS += -lopencv_world341

win32: LIBS += -LC:/boost_1_66_0/stage/x64/lib/
win32:  LIBS += -lboost_filesystem-vc141-mt-x64-1_66
win32:  LIBS += -lboost_regex-vc141-mt-x64-1_66
