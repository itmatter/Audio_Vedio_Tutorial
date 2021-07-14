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


void MainWindow::on_recordBtn_clicked() {
    if (!_vedioThread) { // 点击了“开始录制视频”
        // 开启线程
        _vedioThread = new VedioThread(this);
        _vedioThread->start();
        connect(_vedioThread, &VedioThread::finished,
        [this]() { // 线程结束
            _vedioThread = nullptr;
            ui->recordBtn->setText("开始录制视频");
        });
        // 设置按钮文字
        ui->recordBtn->setText("结束录制视频");
    } else { // 点击了“结束录制视频”
        // 结束线程
        _vedioThread->setStop(true);
        _vedioThread->requestInterruption();
        _vedioThread = nullptr;
        // 设置按钮文字
        ui->recordBtn->setText("开始录制视频");
    }
}
