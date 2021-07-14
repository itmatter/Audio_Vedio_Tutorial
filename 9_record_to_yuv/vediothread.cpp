#include "vediothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


// Mac系统下使用命令行查看录音设备
// ffmpeg -f avfoundation -list_devices true -i ''
//
// AVFoundation video devices:
// [0] FaceTime HD Camera
// [1] Capture screen 0
// AVFoundation audio devices:
// [0] Built-in Microphone
//
//
// 格式名称，设备名称
#ifdef Q_OS_WIN
    //# 查看dshow支持的设备
    //ffmpeg -f dshow -list_devices true -i dummy
    #define FMT_NAME "dshow"
    //    #define DEVICE_NAME "audio=耳机 (靓模袭地球 Hands-Free AG Audio)"
    #define DEVICE_NAME "audio=麦克风 (Realtek High Definition Audio)"
    #define FILEPATH "G:/Resource/"
    #define FILENAME "record_to_pcm.pcm"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME "0"
    #define FILEPATH "/Users/lumi/Desktop/"
    #define FILENAME "record_to_yuv.yuv"

    #define VEDIO_SIZE "1280x720"
    #define FRAMERATE "30"
    #define PIXEL_FORMAT "uyvy422"

#endif

VedioThread::VedioThread(QObject *parent) : QThread(parent) {
//    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &VedioThread::finished,
            this, &VedioThread::deleteLater);
}


VedioThread::~VedioThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}



void VedioThread::run() {
    qDebug() << this << "开始执行----------";
    // 获取输入格式对象
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }
    qDebug() << "fmt : " << fmt;
    // 格式上下文（将来可以利用上下文操作设备）


    AVFormatContext *ctx = nullptr;


    AVDictionary *options = NULL;
    av_dict_set(&options, "video_size", VEDIO_SIZE, 0);
    av_dict_set(&options, "framerate", FRAMERATE, 0);
    av_dict_set(&options, "pixel_format", PIXEL_FORMAT , 0);

    // 打开设备
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "打开设备失败" << errbuf;
        return;
    }
    qDebug() << "ret : " << ret;

    // 文件名
    QString filename = FILEPATH;
    filename += FILENAME;
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;
        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }
    // 数据包
    AVPacket pkt;
    while (!isInterruptionRequested()) {
        // 不断采集数据
        ret = av_read_frame(ctx, &pkt);
        if (ret == 0) { // 读取成功
            // 将数据写入文件
            file.write((const char *) pkt.data, pkt.size);
        } else if (ret == AVERROR(EAGAIN)) { // 资源临时不可用
            continue;
        } else { // 其他错误
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }
    }
    // 释放资源
    // 关闭文件
    file.close();
    // 关闭设备
    avformat_close_input(&ctx);
    qDebug() << this << "正常结束----------";
}

void VedioThread::setStop(bool stop) {
    _stop = stop;
}


