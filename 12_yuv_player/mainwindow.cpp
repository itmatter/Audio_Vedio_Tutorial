#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>


#ifdef Q_OS_WIN
#define FILENAME "G:/BigBuckBunny_CIF_24fps.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 352
#define IMG_H 288
#define FRAME_RATE 24
#else
#define FILENAME "/Users/liliguang/Desktop/record_to_yuv.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_YV12
#define IMG_W 1280
#define IMG_H 720
#define FRAME_RATE 30
#endif




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    double scale = 1;
    if (IMG_W > 500) {
        scale = 0.5;
    }

    int marginX = 100;
    int marginY = 50;
    int btnWeight = 180;
    int btnHeight = 100;

    int windowW = IMG_W + 2 * marginX;
    int windowH = IMG_H + 3 * marginY + btnHeight;

    this->setFixedSize(QSize(windowW * scale,  windowH * scale));
    _yuvPlayer = new YuvPlayer(this);
    _yuvPlayer->setGeometry(marginX * scale , marginY * scale , IMG_W * scale , IMG_H * scale );

    int playBtnX = marginX;
    int playBtnY = IMG_H + marginY * 2;
    int playBtnW = btnWeight;
    int playBtnH = btnHeight;
    ui->playBtn->setGeometry(playBtnX * scale, playBtnY * scale, playBtnW * scale, playBtnH * scale);

    int stopBtnX = IMG_W + marginX - btnWeight;
    int stopBtnY = IMG_H + marginY * 2;
    int stopBtnW = btnWeight;
    int stopBtnH = btnHeight;
    ui->stopBtn->setGeometry(stopBtnX * scale, stopBtnY * scale, stopBtnW * scale, stopBtnH * scale);


}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_playBtn_clicked() {
    // 播放， 接受一个YUV结构体
    YuvParams yuvparam;
    yuvparam.fileName = FILENAME;
    yuvparam.width = IMG_W;
    yuvparam.height = IMG_H;
    yuvparam.framerate = FRAME_RATE;
    yuvparam.pixelFormat = PIXEL_FORMAT;
    yuvparam.timeInterval = (1 * 1000 / yuvparam.framerate);

    if (_yuvPlayer->isStop()) {
        ui->playBtn->setText("暂停");
        _yuvPlayer->play(yuvparam);
    } else {
        if(_yuvPlayer->isPlaying()) {
            ui->playBtn->setText("播放");
            _yuvPlayer->pause();
        } else {
            ui->playBtn->setText("暂停");
            _yuvPlayer->play(yuvparam);
        }
    }


}

void MainWindow::on_stopBtn_clicked() {
    ui->playBtn->setText("播放");
    _yuvPlayer->stop();
}
