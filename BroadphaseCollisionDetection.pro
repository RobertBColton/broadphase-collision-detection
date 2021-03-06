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
    MainWindow.cpp

HEADERS  += \
    AABB.hpp \
    Broadphase.hpp \
    MainWindow.hpp \
    PruneSweep.hpp \
    Quadtree.hpp \
    SpatialHash.hpp

FORMS    +=
