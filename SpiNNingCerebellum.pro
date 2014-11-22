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
    dataprovider.cpp

HEADERS += \
    settingsdialog.h \
    console.h \
    qcustomplot.h \
    mainwindow.h \
    serialTab.h \
    plotTab.h \
    dataprovider.h

FORMS += \
    settingsdialog.ui \
    mainwindow.ui \
    serialtab.ui \
    plottab.ui

RESOURCES += \
    icons.qrc
