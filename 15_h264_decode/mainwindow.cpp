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


void MainWindow::on_decodeBtn_clicked() {
    _h264DecodeThread = new H264DecodeThread(this);
    _h264DecodeThread->start();
}
