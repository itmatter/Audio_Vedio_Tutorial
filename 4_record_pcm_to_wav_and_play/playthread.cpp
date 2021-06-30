#include "playthread.h"
#include <QDebug>



PlayThread::PlayThread(QObject *parent) : QThread(parent) {
//    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &PlayThread::finished,
            this, &PlayThread::deleteLater);
}


PlayThread::~PlayThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}




void PlayThread::run() {
    qDebug() <<  "run";
    sleep(2);
}

void PlayThread::setStop(bool stop) {
    _stop = stop;
}


