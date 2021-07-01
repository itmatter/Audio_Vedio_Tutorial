#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
extern "C" {
#include <libavdevice/avdevice.h>
#include "SDL2/SDL.h"
}

int main(int argc, char *argv[])
{
    qDebug() << "SDL_Init : "<<SDL_Init(SDL_INIT_AUDIO);


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // 初始化libavdevice 并注册所有输入和输出设备 ， 只需要注册一次就可以了
    avdevice_register_all();
    return a.exec();
}
