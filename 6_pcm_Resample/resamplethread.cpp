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
//    outputFile.filename = "G:/Resource/record_to_pcm.pcm";
    outputFile.filename = "/Users/liliguang/Desktop/record_to_pcm.pcm";
    outputFile.sampleFmt = AV_SAMPLE_FMT_S16;
    outputFile.sampleRate = 44100;
    outputFile.chLayout = AV_CH_LAYOUT_STEREO;

    //输入音频文件上下文
    ResampleAudioSpec inputFile;
//    inputFile.filename = "G:/Resource/record_to_pcm.pcm";
    inputFile.filename = "/Users/liliguang/Desktop/record_to_pcm2.pcm";
    inputFile.sampleFmt = AV_SAMPLE_FMT_S16;//因为我的文件是从win系统下录制的， 所以对应的格式ffplay -ar 44100 -ac 2 -f s16le G:/Resource/record_to_pcm.pcm
    inputFile.sampleRate = 48000;
    inputFile.chLayout = AV_CH_LAYOUT_STEREO;

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

    // 前置变量声明 (因为goto语句后面不能定义变量)
    // 初始化上下文ret
    int initRet = 0;
    // 缓冲区大小
    int outBufferSize = 0;
    int inBufferSize = 0;
    // 文件操作
    QFile outFile(outputFile.filename);
    QFile inFile(inputFile.filename);
    int outOpenFileRet = 0;
    int inOpenFileRet = 0;
    // 读取输出文件数据的大小
    int outDataLen = 0;
    // 重采样ret
    int swrConvertRet = 0;


    // 创建上下文
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
    // 初始化上下文
    initRet = swr_init(swrCtx);
    CHECK_IF_ERROR_BUF_END(initRet, "swr_init outBufferSize");

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

    // 文件操作
    outOpenFileRet = !outFile.open(QFile::ReadOnly);
    inOpenFileRet = !inFile.open(QFile::WriteOnly);
    CHECK_IF_ERROR_BUF_END(outOpenFileRet, "outFile.open outputFile.filename");
    CHECK_IF_ERROR_BUF_END(inOpenFileRet, "inFile.open inputFile.filename");


    qDebug() << "av_get_channel_layout_nb_channels(outputFile.chLayout)" << av_get_channel_layout_nb_channels(outputFile.chLayout) ;
    qDebug() << "av_get_bytes_per_sample(outputFile.sampleFmt)" << av_get_bytes_per_sample(outputFile.sampleFmt);

    //重采样
    while ((outDataLen = outFile.read((char *) outData[0], outlinesize)) > 0) {
        // 读取的样本数量

        // 重采样(返回值转换后的样本数量) swrConvertRet:每个通道输出的样本数，错误时为负值
        swrConvertRet = swr_convert(swrCtx,
                                    outData,
                                    out_nb_samples,
                                    (const uint8_t **) inData,
                                    outDataLen / ( av_get_channel_layout_nb_channels(outputFile.chLayout) * av_get_bytes_per_sample(outputFile.sampleFmt) )  );

        qDebug() << "swrConvertRet" << outDataLen <<  swrConvertRet << out_nb_samples << in_nb_samples;
        CHECK_IF_ERROR_BUF_END(swrConvertRet <= 0, "swr_convert");


        inFile.write((char *) inData[0], swrConvertRet *  (av_get_channel_layout_nb_channels(inputFile.chLayout) * av_get_bytes_per_sample(inputFile.sampleFmt))  );
    }

    // 检查一下输出缓冲区是否还有残留的样本（已经重采样过的，转换过的）
    while ((swrConvertRet = swr_convert(swrCtx, outData, out_nb_samples, nullptr, 0)) > 0)
    {
        inFile.write((char *) inData[0], swrConvertRet * (av_get_channel_layout_nb_channels(inputFile.chLayout) * av_get_bytes_per_sample(inputFile.sampleFmt)) );
    }


end:
    // 释放资源
    // 关闭文件
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
