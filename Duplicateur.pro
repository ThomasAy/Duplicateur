#-------------------------------------------------
#
# Project created by QtCreator 2014-05-19T14:20:31
#
#-------------------------------------------------

QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Duplicateur
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    Copier.cpp \
    Progression.cpp

HEADERS  += MainWindow.h \
    Copier.h \
    Progression.h

FORMS    += MainWindow.ui \
    Progression.ui
