QT += core gui widgets printsupport network serialport sql

CONFIG += c++11
TARGET = SpiNNingCerebellum
TEMPLATE = app

SOURCES += \
    main.cpp \
    settingsdialog.cpp \
    console.cpp \
    qcustomplot.cpp \
    mainwindow.cpp \
    serialTab.cpp \
    plotTab.cpp \
    dataprovider.cpp \
    can/CanInterface.cpp \
    can/KvaserCanInterface.cpp \
    can/muscleDriverCANInterface.cpp \
    canTab.cpp \
    can/main-old.cpp \
    dbconnection.cpp \
    canDataProvider.cpp \
    controlTab.cpp \
    dbdata.cpp

HEADERS += \
    settingsdialog.h \
    console.h \
    qcustomplot.h \
    mainwindow.h \
    serialTab.h \
    plotTab.h \
    dataprovider.h \
    can/CanInterface.h \
    can/KvaserCanInterface.h \
    can/muscleDriverCANInterface.h \
    canTab.h \
    dbconnection.h \
    canDataProvider.h \
    controlTab.h \
    dbdata.h

FORMS += \
    settingsdialog.ui \
    mainwindow.ui \
    serialtab.ui \
    plottab.ui \
    cantab.ui \
    controltab.ui

RESOURCES += \
    icons.qrc

LIBS += -lcanlib
