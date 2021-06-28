#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
extern "C" {
#include <libavcodec/avcodec.h>
}

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    qDebug() << av_version_info();
    qDebug() << "end";


    return a.exec();
}
