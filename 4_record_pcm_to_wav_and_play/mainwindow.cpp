#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wavhander.h"
#include <QtDebug>

extern "C" {
#include <libavdevice/avdevice.h>
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_recordBtn_clicked() {
    // 调动
    qDebug() << "on_recordBtn_clicked";
    if (!_audioThread) { // 点击了“开始录音”
        // 开启线程
        _audioThread = new AudioThread(this);
        _audioThread->start();
        connect(_audioThread, &AudioThread::finished,
        [this]() { // 线程结束
            _audioThread = nullptr;
            ui->recordBtn->setText("开始录音");
        });
        // 设置按钮文字
        ui->recordBtn->setText("结束录音");
    } else { // 点击了“结束录音”
        // 结束线程
        _audioThread->setStop(true);
        _audioThread->requestInterruption();
        _audioThread = nullptr;
        // 设置按钮文字
        ui->recordBtn->setText("开始录音");
    }
}




void MainWindow::on_playBtn_clicked() {
    qDebug() << "on_playBtn_clicked";
    // 调动
    if (!_audioThread) { // 点击了“播放”
        // 开启线程
        _playThread = new PlayThread(this);
        _playThread->start();
        connect(_playThread, &PlayThread::finished,
        [this]() { // 线程结束
            _playThread = nullptr;
            ui->playBtn->setText("播放");
        });
        // 设置按钮文字
        ui->playBtn->setText("停止");
    } else { // 点击了“结束录音”
        // 结束线程
        _playThread->setStop(true);
        _playThread->requestInterruption();
        _playThread = nullptr;
        // 设置按钮文字
        ui->playBtn->setText("播放");
    }
}
