QT += core gui widgets printsupport network serialport

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
    can/main2.cpp \
    canTab.cpp

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
    can/UserInterface.h \
    canTab.h

FORMS += \
    settingsdialog.ui \
    mainwindow.ui \
    serialtab.ui \
    plottab.ui \
    cantab.ui

RESOURCES += \
    icons.qrc

LIBS += -lcanlib
