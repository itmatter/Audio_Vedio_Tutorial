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


void MainWindow::on_h264EncodeBtn_clicked() {
    _h264EncodeThread = new H264EncodeThread(this);
    _h264EncodeThread->start();
}
