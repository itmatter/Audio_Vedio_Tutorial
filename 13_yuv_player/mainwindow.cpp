#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>


#define FILENAME "G:/BigBuckBunny_CIF_24fps.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 352
#define IMG_H 288
#define FRAME_RATE 24



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 主窗口大小 ：800* 600
    this->setFixedSize(QSize(800, 600));

    _yuvPlayer = new YuvPlayer(this);
    _yuvPlayer->setGeometry(200, 50, 400, 400);

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
