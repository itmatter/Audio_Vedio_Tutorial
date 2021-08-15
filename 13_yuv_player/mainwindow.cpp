#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_playBtn_clicked() {
    // 调动
    qDebug() << "on_recordBtn_clicked";
    if (!_playThread) { // 点击了“播放”
        qDebug() << "播放";
        // 开启线程
        _playThread = new Playthread(this);
        _playThread->start();
        connect(_playThread, &Playthread::finished,
        [this]() { // 线程结束
            _playThread = nullptr;
            ui->playBtn->setText("播放");
        });
        // 设置按钮文字
        ui->playBtn->setText("停止");
    } else { // 点击了“结束录音”
        // 结束线程
        qDebug() << "结束";
        _playThread->setStop(true);
        _playThread->requestInterruption();
        _playThread = nullptr;
        // 设置按钮文字
        ui->playBtn->setText("播放");
    }
}

void MainWindow::on_stopBtn_clicked() {
    QWidget *widget = new QWidget(this);
    qDebug() << "on_stopBtn_clicked" << widget->winId();

}
