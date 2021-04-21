#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wavhander.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_recordBtn_clicked()
{


    //调用函数
    WAVHeader header;
    header.numChannel = 2;
    header.sampleRate = 44100;
    header.bitsPerSample = 32; //

    WAVhander::pcm_To_wav(header,
                          (char *)"/Users/lumi/Desktop/record_to_pcm.pcm",
                          (char *)"/Users/lumi/Desktop/pcm_To_wav.wav" );

    return;

}
