#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wavhander.h"
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_recordBtn_clicked() {
    //调用函数
    WAVHeader header;
    header.numChannel = 2;
    header.sampleRate = 44100;
    header.bitsPerSample = 16;//win 是s16le
    //header.bitsPerSample = 32;//mac 是f32le
    qDebug() << "PCM地址：" << pcmUrl; //G:\Resourcerecord_to_pcm.pcm
    qDebug() << "WAV地址：" << wavUrl; //G:\Resourcerecord_to_pcm.wav
    WAVhander::pcm_To_wav(header, pcmUrl.toLatin1().data(), wavUrl.toLatin1().data() );
    return;
}

void MainWindow::on_PCNEdit_textChanged(const QString &arg1) {
    pcmUrl = arg1;
    qDebug() << "PCM地址：" << pcmUrl;
}

void MainWindow::on_WAVEdit_textChanged(const QString &arg1) {
    wavUrl = arg1;
    qDebug() << "WAV地址：" << wavUrl;
}
