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
    wavhander.cpp

HEADERS += \
    audiothread.h \
    mainwindow.h \
    wavhander.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


macx {
    # 设置头文件路径
    FFMPEG_HOME = /usr/local/Cellar/ffmpeg/4.3.2_3
    # 配置权限相关
    QMAKE_INFO_PLIST = mac/Info.plist
}

win32 {
    FFMPEG_HOME = G:\ffmpeg-4.3.2
}


INCLUDEPATH += $${FFMPEG_HOME}/include

LIBS += -L $${FFMPEG_HOME}/lib \
-lavcodec \
-lavdevice \
-lavfilter \
-lavformat \
-lavutil \
-lpostproc \
-lswscale \
-lswresample
