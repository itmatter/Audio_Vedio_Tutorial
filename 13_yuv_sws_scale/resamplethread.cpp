#include "resamplethread.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));


#define CHECK_IF_ERROR_BUF_END(ret, funcStr) \
    if (ret) { \
        ERROR_BUF(ret); \
        qDebug() << #funcStr << " error :" << errbuf; \
        goto end; \
    }



#ifdef Q_OS_WIN
    #define IN_PCM_FILEPATH "G:/Resource/record_to_pcm.pcm"
    #define OUT_PCM_FILEPATH "G:/Resource/record_to_pcm2.pcm"
#else
    #define IN_PCM_FILEPATH "/Users/liliguang/Desktop/record_to_pcm.pcm"
    #define OUT_PCM_FILEPATH "/Users/liliguang/Desktop/record_to_pcm2.pcm"
#endif



ResampleThread::ResampleThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &ResampleThread::finished,
            this, &ResampleThread::deleteLater);
}

ResampleThread::~ResampleThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}



typedef struct {
    const char *filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} ResampleAudioSpec;



/**
 * 重采样
 * 1.swr_alloc_set_opts
 * 2.swr_init
 * 3.av_samples_alloc_array_and_samples
 * 4.swr_convert
 *
 *
 * 1.创建上下文
 * 2.初始化上下文
 * 3.创建输出缓冲区
 * 4.创建输入缓冲区
 * 5.重采样
 *
 */




void audio_resampleS (ResampleAudioSpec inSpec, ResampleAudioSpec outSpec)  {
    // 前置变量声明 (因为goto语句后面不能定义变量)

    // 初始化上下文ret
    int initRet = 0;

    // 缓冲区大小
    int inBufferSize = 0;
    int outBufferSize = 0;

    // 文件操作
    QFile inFile(inSpec.filename);
    QFile outFile(outSpec.filename);

    int inOpenFileRet = 0;
    int outOpenFileRet = 0;

    // 读取输出文件数据的大小
    int inDataLen = 0;

    // 每个样本的总大小
    int inBytesPerSample = ( av_get_channel_layout_nb_channels(inSpec.chLayout) * av_get_bytes_per_sample(inSpec.sampleFmt) )  ;
    int outBytesPerSample = (av_get_channel_layout_nb_channels(outSpec.chLayout) * av_get_bytes_per_sample(outSpec.sampleFmt))  ;

    // 重采样ret
    int swrConvertRet = 0;



    //重采样目标 44100 ==> 48000
    /*
     inSampleRate (48000)               in_nb_samples (输入缓冲区大小)
     ------------------------   =   ------------------------
     outSampleRate (44100)              out_nb_samples (输出缓冲区大小)

     输出缓冲区大小计算公式 out_nb_samples = outSampleRate * in_nb_samples / inSampleRate
     输入缓冲区大小计算公式 in_nb_samples = inSampleRate * out_nb_samples / outSampleRate
     */


    //输入缓冲区
    uint8_t **inData = nullptr;
    int inlinesize = 0;
    int in_nb_channels = av_get_channel_layout_nb_channels(inSpec.chLayout);
    int in_nb_samples = 1024 ;
    enum AVSampleFormat in_sample_fmt = inSpec.sampleFmt;
    int in_align = 0;


    //输出缓冲区
    uint8_t **outData = nullptr;
    int outlinesize = 0;
    int out_nb_channels = av_get_channel_layout_nb_channels(outSpec.chLayout);
    int out_nb_samples = av_rescale_rnd(outSpec.sampleRate, in_nb_samples, inSpec.sampleRate, AV_ROUND_UP);
    enum AVSampleFormat out_sample_fmt = outSpec.sampleFmt;
    int out_align = 0;


    qDebug() << "输入缓冲区" << inSpec.sampleRate << in_nb_samples;
    qDebug() << "输出缓冲区" << outSpec.sampleRate << out_nb_samples;




    //===================================================================
    //===================================================================
    //===========================   主要步骤    ===========================
    //===================================================================
    //===================================================================

    // 1. 创建上下文
    SwrContext *swrCtx = swr_alloc_set_opts(nullptr,
                                            outSpec.chLayout,
                                            outSpec.sampleFmt,
                                            outSpec.sampleRate,
                                            inSpec.chLayout,
                                            inSpec.sampleFmt,
                                            inSpec.sampleRate,
                                            0,
                                            nullptr
                                           );
    CHECK_IF_ERROR_BUF_END(!swrCtx, "swr_alloc_set_opts");

    // 2. 初始化上下文
    initRet = swr_init(swrCtx);
    CHECK_IF_ERROR_BUF_END(initRet, "swr_init");



    // 3. 输入缓冲区
    qDebug() << "输入outBufferRet" << inData << inlinesize << in_nb_channels << in_nb_samples << in_sample_fmt;
    inBufferSize = av_samples_alloc_array_and_samples (&inData,
                   &inlinesize,
                   in_nb_channels,
                   in_nb_samples,
                   in_sample_fmt,
                   in_align);
    CHECK_IF_ERROR_BUF_END(inBufferSize < 0, "av_samples_alloc_array_and_samples inBufferSize");


    // 4. 输出缓冲区
    qDebug() << "输出outBufferRet" << outData << outlinesize << out_nb_channels << out_nb_samples << out_sample_fmt;
    outBufferSize = av_samples_alloc_array_and_samples ( &outData,
                    &outlinesize,
                    out_nb_channels,
                    out_nb_samples,
                    out_sample_fmt,
                    out_align);
    CHECK_IF_ERROR_BUF_END(outBufferSize < 0, "av_samples_alloc_array_and_samples outBufferSize");


    // 文件操作
    inOpenFileRet = !inFile.open(QFile::ReadOnly);
    outOpenFileRet = !outFile.open(QFile::WriteOnly);

    CHECK_IF_ERROR_BUF_END(inOpenFileRet, "inFile.open inSpec.filename");
    CHECK_IF_ERROR_BUF_END(outOpenFileRet, "outFile.open outSpec.filename");

    qDebug() << "av_get_channel_layout_nb_channels(outSpec.chLayout)" << av_get_channel_layout_nb_channels(outSpec.chLayout) ;
    qDebug() << "av_get_bytes_per_sample(outSpec.sampleFmt)" << av_get_bytes_per_sample(outSpec.sampleFmt);

    // 5.重采样
    while ((inDataLen = inFile.read((char *) inData[0], inlinesize)) > 0) {
        // 读取的样本数量
        qDebug() << "in_nb_samples" << in_nb_samples << "out_nb_samples" << out_nb_samples ;
        qDebug() << "inDataLen" << (inDataLen / inBytesPerSample) ;

        // 重采样(返回值转换后的样本数量) swrConvertRet:每个通道输出的样本数，错误时为负值
        swrConvertRet = swr_convert(swrCtx,
                                    outData,
                                    out_nb_samples,
                                    (const uint8_t **) inData,
                                    in_nb_samples);

        // qDebug() << "swrConvertRet" << outDataLen <<  swrConvertRet << out_nb_samples << in_nb_samples;
        CHECK_IF_ERROR_BUF_END(swrConvertRet <= 0, "swr_convert");


        // 写入文件， 转换得出swrConvertRet每个通道样本数， 写入数据 = swrConvertRet *  每个样本的总大小， 写入到outData中
        outFile.write((char *) outData[0], swrConvertRet * outBytesPerSample);
    }

    // 检查一下输出缓冲区是否还有残留的样本（已经重采样过的，转换过的）
    while ((swrConvertRet = swr_convert(swrCtx, outData, out_nb_samples, nullptr, 0)) > 0) {
        outFile.write((char *) outData[0], swrConvertRet * outBytesPerSample );
    }



end:
    // 关闭文件，释放资源
    inFile.close();
    outFile.close();

    // 释放输入缓冲区
    if (inData) {
        av_freep(&inData[0]);
    }
    av_freep(&inData);

    // 释放输出缓冲区
    if (outData) {
        av_freep(&outData[0]);
    }
    av_freep(&outData);

    // 释放重采样上下文
    swr_free(&swrCtx);
}





void ResampleThread::run() {
    // 源文件 ==》输入缓冲区 ==》 输出缓冲区 ==》 输出文件

    //输入音频文件上下文
    ResampleAudioSpec inSpec;
    inSpec.filename = IN_PCM_FILEPATH;
    inSpec.sampleFmt = AV_SAMPLE_FMT_S16;
    inSpec.sampleRate = 44100;
    inSpec.chLayout = AV_CH_LAYOUT_STEREO;

    //输出音频文件上下文
    ResampleAudioSpec outSpec;
    outSpec.filename = OUT_PCM_FILEPATH;
    outSpec.sampleFmt = AV_SAMPLE_FMT_S16;
    outSpec.sampleRate = 48000;
    outSpec.chLayout = AV_CH_LAYOUT_STEREO;

    audio_resampleS(inSpec, outSpec);
}



