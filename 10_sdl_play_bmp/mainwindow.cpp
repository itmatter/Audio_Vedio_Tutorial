#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>

#include <SDL2/SDL.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);



}



MainWindow::~MainWindow() {
    delete ui;
}


// 格式名称，设备名称
#ifdef Q_OS_WIN
#define FILENAME "G:/Resource/in.bmp"
#else
#define FILENAME "/Users/liliguang/Desktop/in.bmp"
#endif


void MainWindow::on_playBtn_clicked() {
    qDebug() << "on_recordBtn_clicked";

    if (!_yuvPlayer) {
        _yuvPlayer = new YUVPlayer(this);
        _yuvPlayer->showBmp((char *)FILENAME);
        _yuvPlayer->setAttribute(Qt::WA_DeleteOnClose);
        qDebug() << "Create";
    } else {
        _yuvPlayer->close();
        _yuvPlayer = nullptr;
         qDebug() << "End";
    }



}
