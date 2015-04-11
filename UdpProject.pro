#-------------------------------------------------
#
# Project created by QtCreator 2015-04-06T10:21:34
#
#-------------------------------------------------

QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpProject
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    keypresseater.cpp \
    receivefiledialog.cpp \
    udpreceiver.cpp \
    udpsender.cpp

HEADERS  += mainwindow.h \
    keypresseater.h \
    receivefiledialog.h \
    udpreceiver.h \
    udpsender.h

FORMS    += mainwindow.ui \
    receivefiledialog.ui

RESOURCES += \
    udpproject.qrc
