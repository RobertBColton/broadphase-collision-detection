#-------------------------------------------------
#
# Project created by QtCreator 2017-04-28T04:57:51
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BroadphaseCollisionDetection
TEMPLATE = app


SOURCES += main.cpp \
    MainWindow.cpp \
    Quadtree.cpp

HEADERS  += \
    AxisAlignedBoundingBox.hpp \
    Broadphase.h \
    Quadtree.h \
    SparseSpatialBroadphase.hpp \
    MainWindow.hpp

FORMS    +=
