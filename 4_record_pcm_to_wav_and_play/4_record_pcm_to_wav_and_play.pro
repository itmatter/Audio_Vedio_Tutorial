QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiothread.cpp \
    main.cpp \
    mainwindow.cpp \
    playthread.cpp \
    wavhander.cpp

HEADERS += \
    audiothread.h \
    mainwindow.h \
    playthread.h \
    wavhander.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


macx {
    # 设置头文件路径
    LIB_HOME = ./Res
    # 配置权限相关
    QMAKE_INFO_PLIST = mac/Info.plist
}

win32 {
    LIB_HOME = ./Res
}


INCLUDEPATH += $${LIB_HOME}/include

LIBS += -L $${LIB_HOME}/lib -lSDL2
LIBS += -L $${LIB_HOME}/lib -lavcodec
LIBS += -L $${LIB_HOME}/lib -lavdevice
LIBS += -L $${LIB_HOME}/lib -lavfilter
LIBS += -L $${LIB_HOME}/lib -lavformat
LIBS += -L $${LIB_HOME}/lib -lavutil
LIBS += -L $${LIB_HOME}/lib -lpostproc
LIBS += -L $${LIB_HOME}/lib -lswscale
LIBS += -L $${LIB_HOME}/lib -lswresample

