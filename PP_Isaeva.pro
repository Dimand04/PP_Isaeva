QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    createconsumtion.cpp \
    createreceipt.cpp \
    createuser.cpp \
    doc.cpp \
    global.cpp \
    main.cpp \
    mainwindow.cpp \
    qpushlabel.cpp

HEADERS += \
    createconsumtion.h \
    createreceipt.h \
    createuser.h \
    doc.h \
    global.h \
    mainwindow.h \
    qpushlabel.h

FORMS += \
    createconsumtion.ui \
    createreceipt.ui \
    createuser.ui \
    doc.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
