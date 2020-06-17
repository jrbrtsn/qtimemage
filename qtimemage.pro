#-------------------------------------------------
#
# Project created by QtCreator 2018-09-03T21:49:33
#
#-------------------------------------------------

QT       += core gui sql network

#CONFIG += qt release
CONFIG += qt debug

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtimemage
TEMPLATE = app

#release: DESTDIR=build/release
debug: DESTDIR=build/debug

OBJECTS_DIR = $${DESTDIR}
MOC_DIR = $${DESTDIR}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
debug: DEFINES += DEBUG

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    ProjectComboBox.cpp \
    ProjectChooser.cpp \
    EventTreeWidget.cpp \
    MonthChooser.cpp \
    WeekChooser.cpp \
    ProjectTreeWidget.cpp \
    ClientTabBar.cpp \
    main.cpp \
    MainWindow.cpp \
    Client.cpp \
    DbRec.cpp \
    UnixSignals.cpp \
    ClientEditor.cpp \
    configItem.cpp \
    Global.cpp \
    MainToolBar.cpp \
    Project.cpp \
    ProjectPage.cpp \
    ProjectEditor.cpp \
    ClientPage.cpp \
    RateEditor.cpp \
    Event.cpp \
    Report.cpp \
    util.cpp \

HEADERS += \
    ProjectComboBox.h \
    ProjectChooser.h \
    EventTreeWidget.h \
    MonthChooser.h \
    WeekChooser.h \
    ProjectTreeWidget.h \
    ClientTabBar.h \
    MainWindow.h \
    qtimemage.h \
    Client.h \
    DbRec.h \
    UnixSignals.h \
    ClientEditor.h \
    configItem.h \
    MainToolBar.h \
    Project.h \
    ProjectPage.h \
    ProjectEditor.h \
    ClientPage.h \
    RateEditor.h \
    Event.h \
    Report.h \

