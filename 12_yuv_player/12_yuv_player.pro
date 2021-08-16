QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    yuvplayer.cpp

HEADERS += \
    mainwindow.h \
    yuvplayer.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


macx {
    # 设置头文件路径
    SDL_HOME = /usr/local/Cellar/sdl2/2.0.14_1
    # 配置权限相关
    QMAKE_INFO_PLIST = mac/Info.plist
}

win32 {
    SDL_HOME = G:\SDL2-2.0.14\x86_64-w64-mingw32
}

INCLUDEPATH += $${SDL_HOME}/include
LIBS += -L $${SDL_HOME}/lib \
-lSDL2 \
-lSDL2_test \
-lSDL2main



