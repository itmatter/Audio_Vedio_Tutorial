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
 */
void ResampleThread::run() {

    //1.创建上下文
    //2.初始化上下文
    //3.创建输出缓冲区
    //4.创建输入缓冲区
    //5.重采样


    //输出音频文件上下文
    ResampleAudioSpec outputFile;
    outputFile.filename = "G:/Resource/record_to_pcm.pcm";
    outputFile.sampleFmt = AV_SAMPLE_FMT_S16;
    outputFile.sampleRate = 44100;
    outputFile.chLayout = AV_CH_LAYOUT_STEREO;

    //输入音频文件上下文
    ResampleAudioSpec inputFile;
    inputFile.filename = "G:/Resource/record_to_pcm.pcm";
    inputFile.sampleFmt = AV_SAMPLE_FMT_S16;//因为我的文件是从win系统下录制的， 所以对应的格式ffplay -ar 44100 -ac 2 -f s16le G:/Resource/record_to_pcm.pcm
    inputFile.sampleRate = 48000;
    inputFile.chLayout = AV_CH_LAYOUT_STEREO;

    SwrContext *swrCtx = swr_alloc_set_opts(nullptr,
                                            outputFile.chLayout,
                                            outputFile.sampleFmt,
                                            outputFile.sampleRate,
                                            inputFile.chLayout,
                                            inputFile.sampleFmt,
                                            inputFile.sampleRate,
                                            0,
                                            nullptr
                                           );

    int initRet = 0;
    initRet = swr_init(swrCtx);



    //处理输入输出缓冲区函数参数
    //uint8_t ***audio_data 数组要填充每个通道的指针, 三个*,
    //理解：
    // *audio_data => audio_data指针                          指针A -> 内存地址
    // **audio_data => 指向 audio_data的指针                   指针B -> 指针A
    // ***audio_data => 指向 指向 audio_data的指针 的指针        指针C -> 指针B



    //重采样目标 44100 ==> 48000
    /*
     inSampleRate (48000)               in_nb_samples (输入缓冲区大小)
     ------------------------   =   ------------------------
     outSampleRate (44100)              out_nb_samples (1024 输出缓冲区大小)

     输出缓冲区大小计算公式 out_nb_samples = outSampleRate * in_nb_samples / inSampleRate
     输入缓冲区大小计算公式 in_nb_samples = inSampleRate * out_nb_samples / outSampleRate
     */


    //输出缓冲区
    uint8_t **outData = nullptr;
    int outlinesize = 0;
    int out_nb_channels = av_get_channel_layout_nb_channels(outputFile.chLayout);
    int out_nb_samples = 1024 ;
    enum AVSampleFormat out_sample_fmt = outputFile.sampleFmt;
    int out_align = 0;


    //输入缓冲区
    uint8_t **inData = nullptr;
    int inlinesize = 0;
    int in_nb_channels = av_get_channel_layout_nb_channels(inputFile.chLayout);
    int in_nb_samples =  av_rescale_rnd(inputFile.sampleRate, out_nb_samples, outputFile.sampleRate, AV_ROUND_UP);//a * b / c
    enum AVSampleFormat in_sample_fmt = inputFile.sampleFmt;
    int in_align = 0;


    qDebug() << "输出缓冲区" << outputFile.sampleRate << out_nb_samples;
    qDebug() << "输入缓冲区" << inputFile.sampleRate << in_nb_samples;


    // 开辟内存空间大小
    int outBufferSize = 0;
    int inBufferSize = 0;

    qDebug() << "输出outBufferRet" << outData << outlinesize << out_nb_channels << out_nb_samples << out_sample_fmt;
    outBufferSize = av_samples_alloc_array_and_samples ( &outData,
                    &outlinesize,
                    out_nb_channels,
                    out_nb_samples,
                    out_sample_fmt,
                    out_align);
    CHECK_IF_ERROR_BUF_END(outBufferSize < 0, "av_samples_alloc_array_and_samples outBufferSize");


    qDebug() << "输入outBufferRet" << inData << inlinesize << in_nb_channels << in_nb_samples << in_sample_fmt;
    inBufferSize = av_samples_alloc_array_and_samples (&inData,
                   &inlinesize,
                   in_nb_channels,
                   in_nb_samples,
                   in_sample_fmt,
                   in_align);
    CHECK_IF_ERROR_BUF_END(inBufferSize < 0, "av_samples_alloc_array_and_samples inBufferSize");




    //打开文件
    QFile outFile(outputFile.filename);
    QFile inFile(inputFile.filename);

    int outOpenFileRet = 0;
    int inOpenFileRet = 0;
    outOpenFileRet = !outFile.open(QFile::ReadOnly);
    inOpenFileRet = !inFile.open(QFile::ReadOnly);
    // 打开文件
    CHECK_IF_ERROR_BUF_END(outOpenFileRet, "outFile.open outputFile.filename");
    CHECK_IF_ERROR_BUF_END(inOpenFileRet, "inFile.open inputFile.filename");







end:
    // 释放资源
//    // 关闭文件
//    inFile.close();
//    outFile.close();

//    // 释放输入缓冲区
    if (inData) {
        av_freep(&inData[0]);
    }

    // 释放输出缓冲区
    if (outData) {
        av_freep(&outData[0]);
    }

//    // 释放重采样上下文
    swr_free(&swrCtx);

}
