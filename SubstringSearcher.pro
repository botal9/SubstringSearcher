#-------------------------------------------------
#
# Project created by QtCreator 2019-01-10T23:28:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SubstringSearcher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        src/main.cpp \
        src/interface/mainwindow.cpp \
        src/library/worker.cpp \
        src/library/indexer.cpp \
        src/library/searcher.cpp \
        src/library/retry.cpp

HEADERS += \
        src/interface/mainwindow.h \
        src/library/worker.h \
        src/library/indexer.h \
        src/library/searcher.h \
        src/library/retry.h

FORMS += \
        src/interface/mainwindow.ui

CONFIG (debug, debug|release) {
    CONFIG += \
        console
}
