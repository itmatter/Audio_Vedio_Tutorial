#include "swsscalethread.h"
#include <QFile>
#include <QDebug>

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>

}


#define SWS_ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));


#define CHECK_END(ret, funcStr) \
    if (ret) { \
        SWS_ERROR_BUF(ret); \
        qDebug() << #funcStr << " error :" << errbuf; \
        goto end; \
    }



SwsScaleThread::SwsScaleThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &SwsScaleThread::finished,
            this, &SwsScaleThread::deleteLater);
}

SwsScaleThread::~SwsScaleThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}





void SwsScaleThread::run() {

    // ffplay -video_size 1280X720 -pixel_format uyvy422 -framerate 30 /Users/liliguang/Desktop/record_to_yuv.yuv
    // 把 uyvy422 转为 yuv420p
    // uyvy422 的数据格式  uyvy uyvy uyvy uyvy   占2个字节
    // yuv420p 的数据格式  yyyy yyyy uu vv       占1.5个字节
    // 整体思路, 一帧的uyvy422 转为 一帧的yuv420p
    // 简单粗暴 : 读取输入源文件, 每次读取一帧的数据, 读取传入buffer缓冲区, 通过核心函数sws_scale, 转换到输出缓冲区, 最后写入目标文件中


    // 输入源
    int srcW = 1280;
    int srcH = 720;
    AVPixelFormat srcFormat = AV_PIX_FMT_UYVY422;
    int srcImageSize = srcW * srcH * 2;

    // 输入源buffer缓冲区
    // 可能遇到的是yuv, 也可能是rgba ,  简单就是
    // srcData[0] -> y 或者 r ;
    // srcData[1] -> u 或者 g ;
    // srcData[2] -> v 或者 b ;
    // srcData[3] -> a
    uint8_t *srcData[4];

    // 可能遇到的是yuv, 也可能是rgba ,  简单就是
    // srclinesizes[0] -> y分量的步长 或者 r分量的步长 ;
    // srclinesizes[1] -> u分量的步长 或者 g分量的步长 ;
    // srclinesizes[2] -> v分量的步长 或者 b分量的步长 ;
    // srclinesizes[3] -> a分量的步长
    int srclinesizes[4];

    // 创建srcData缓冲区Ret
    int srcDataRet;
    // 输入源文件
    const char *srcFilePath = "/Users/liliguang/Desktop/srcYuv.yuv";


    // 输出源
    int dstW = 1280;
    int dstH = 720;
    AVPixelFormat dstFormat = AV_PIX_FMT_YUV420P;
    int dsrImageSize = dstW * dstH * 1.5;

    // 输出源buffer缓冲区
    uint8_t *dstData[4];
    int dstlinesizes[4];
    int dstDataRet;

    const char *dstFilePath = "/Users/liliguang/Desktop/dstYuv.yuv";

    QFile srcFile(srcFilePath);
    QFile dstFile(dstFilePath);

    int openSrcFileRet;
    int openDstFileRet;



    struct SwsContext *context;
    int swsScaleRet;

    // 第一步 : 文件操作
    openSrcFileRet = srcFile.open(QFile::ReadOnly);
    CHECK_END(!openSrcFileRet, "srcFile open");

    openDstFileRet = dstFile.open(QFile::WriteOnly);
    CHECK_END(!openDstFileRet, "dstFile open");



    // 第二步 : 创建上下文,  后面四个参数参照官方Demo
    context = sws_getContext(
                srcW, srcH, srcFormat,
                dstW, dstH, dstFormat,
                SWS_BILINEAR, NULL,NULL,NULL
                );
    CHECK_END(!context, "sws_getContext");



    // 第三步 : 创建buffer缓冲区
    srcDataRet =  av_image_alloc(srcData, srclinesizes, srcW, srcH, srcFormat, 16);
    CHECK_END(!srcDataRet, "srcData av_image_alloc");
    qDebug() <<"srcDataRet : " << srcDataRet ;

    dstDataRet =  av_image_alloc(dstData, dstlinesizes, dstW, dstH, dstFormat, 16);
    CHECK_END(!dstDataRet, "dstData av_image_alloc");
    qDebug() <<"dstDataRet : " << dstDataRet ;


    qDebug() <<"开始读取文件" ;

    int readRet;
    int writeRet;
    while( (readRet = srcFile.read((char *)srcData[0], srcImageSize)) > 0) {
        qDebug() << "readRet : " << readRet;
        // 转换
        swsScaleRet = sws_scale(context, srcData, srclinesizes, 0, srcH, dstData, dstlinesizes );
        qDebug() << "swsScaleRet : " << swsScaleRet;

        // 写入文件
        writeRet = dstFile.write((char *)dstData[0], dsrImageSize);
        CHECK_END(!writeRet, "dstFile write");


    }
    qDebug() << "readRet : " << readRet;
    qDebug() <<"结束读取文件" ;



end:
    // 关闭文件，释放资源
    srcFile.close();
    dstFile.close();

    // 释放输入缓冲区
    av_freep(&srcData);
    av_freep(&dstData);

    // 释放重采样上下文
    sws_freeContext(context);
}
