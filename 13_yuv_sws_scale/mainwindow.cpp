#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_reSampleBtn_clicked() {
    qDebug() << "on_reSampleBtn_clicked";
    // 音频重采样
//    _resampleThread = new ResampleThread(this);
//    _resampleThread->start();


}

void MainWindow::on_swsScaleBtn_clicked()
{
    qDebug() << "on_swsScaleBtn_clicked";

    // 像素格式转换
    _swsScaleThread = new SwsScaleThread(this);
    _swsScaleThread->start();
}
