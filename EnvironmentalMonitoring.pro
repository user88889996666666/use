QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EnvironmentalMonitoring
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sensormanager.cpp \
    alarmsystem.cpp \
    datalogger.cpp \
    hardwareinterface.cpp

HEADERS += \
    mainwindow.h \
    sensormanager.h \
    alarmsystem.h \
    datalogger.h \
    hardwareinterface.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
