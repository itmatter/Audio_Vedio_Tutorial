#include "aacEncodeThread.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
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
    #define OUT_AAC_FILEPATH "G:/Resource/pcm_to_aac.aac"
    #define SAMPLE_FMT AV_SAMPLE_FMT_S16
#else
    #define IN_PCM_FILEPATH "/Users/liliguang/Desktop/record_to_pcm.pcm"
    #define OUT_AAC_FILEPATH "/Users/liliguang/Desktop/pcm_to_aac.aac"
    #define SAMPLE_FMT AV_SAMPLE_FMT_FLTP
#endif



AACEncodeThread::AACEncodeThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AACEncodeThread::finished,
            this, &AACEncodeThread::deleteLater);
}

AACEncodeThread::~AACEncodeThread() {
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
} AudioSpec;



/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec,
                            enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt) {
            return 1;
        }
        p++;
    }
    return 0;
}


// 音频编码
// 返回负数：中途出现了错误
// 返回0：编码操作正常完成
static int encode(AVCodecContext *ctx,
                  AVFrame *frame,
                  AVPacket *pkt,
                  QFile &outFile) {
    // 发送数据到编码器
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    // 不断从编码器中取出编码后的数据
    // while (ret >= 0)
    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            //  output is not available in the current state - user must try to send input
            // 继续读取数据到frame，然后送到编码器
            return 0;
        } else if (ret < 0) { // 其他错误
            return ret;
        }

        // 成功从编码器拿到编码后的数据
        // 将编码后的数据写入文件
        outFile.write((char *) pkt->data, pkt->size);

        // 释放pkt内部的资源
        av_packet_unref(pkt);
    }

}




void AACEncodeThread::run() {
    qDebug() << "AACEncodeThread run ";

    // 输入输出文件
    const char *infilename;
    const char *outfilename;

    // 编码器
    const AVCodec *codec;
    // 编码器上下文
    AVCodecContext *codecCtx = nullptr;
    // 源文件数据源存储结构指针
    AVFrame *frame = nullptr;
    // 编码文件数据源存储结构指针
    AVPacket *pkt = nullptr;

    int check_sample_fmt_Ret;
    int avcodec_open2_Ret;
    int av_frame_get_buffer_Ret;

    int infileOpen_Ret;
    int outfileOpen_Ret;

    int readFile_Ret;
    int encode_ret;


    infilename = IN_PCM_FILEPATH;
    outfilename = OUT_AAC_FILEPATH;

    QFile inFile(infilename);
    QFile outFile(outfilename);


    // 编码器
    // codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    codec = avcodec_find_encoder_by_name("libfdk_aac"); // 用查找编码器的名称的方式。 默认的可能找到的不一样

    CHECK_IF_ERROR_BUF_END(!codec, "avcodec_find_encoder");

    // 创建编码器上下文
    codecCtx = avcodec_alloc_context3(codec);
    CHECK_IF_ERROR_BUF_END(!codecCtx, "avcodec_alloc_context3");

    // 设置编码器上下文参数
    codecCtx->sample_rate = 44100;
    codecCtx->sample_fmt = SAMPLE_FMT; // planner格式
    codecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    codecCtx->channels = av_get_channel_layout_nb_channels(codecCtx->channel_layout);

    // 不同的比特率影响不同的编码大小.
    // codecCtx->bit_rate = (44100 * 32 * codecCtx->channels);
    // codecCtx->bit_rate = 64000;

    // 比特率
    codecCtx->bit_rate = 32000;
    // 规格
    codecCtx->profile = FF_PROFILE_AAC_HE_V2;


    // 检查编码器支持的样本格式
    check_sample_fmt_Ret = check_sample_fmt(codec, codecCtx->sample_fmt);
    CHECK_IF_ERROR_BUF_END(!check_sample_fmt_Ret, "check_sample_fmt");

    // 打开编码器
    avcodec_open2_Ret = avcodec_open2(codecCtx, codec, nullptr);
    CHECK_IF_ERROR_BUF_END(avcodec_open2_Ret, "avcodec_open2");

    // 打开源文件
    infileOpen_Ret = !inFile.open(QFile::ReadOnly);
    CHECK_IF_ERROR_BUF_END(infileOpen_Ret, "sourceFile.open");

    // 打开源文件
    outfileOpen_Ret = !outFile.open(QFile::WriteOnly);
    CHECK_IF_ERROR_BUF_END(outfileOpen_Ret, "sourceFile.outFile");

    // 创建输出Packet
    pkt = av_packet_alloc();
    CHECK_IF_ERROR_BUF_END(!pkt, "av_packet_alloc");

    // 创建AVFrame结构体本身
    frame = av_frame_alloc();
    CHECK_IF_ERROR_BUF_END(!frame, "av_frame_alloc");

    // 设置frame必要信息
    frame->format = codecCtx->sample_fmt;//样本格式
    frame->nb_samples = codecCtx->frame_size;//每个声道的样本数量大小
    frame->channel_layout = codecCtx->channel_layout; //声道布局


    // 为音频或视频数据分配新的缓冲区。
    // 在调用此函数之前，必须在框架上设置以下字段：
    // - 格式（视频的像素格式，音频的样本格式）
    // - 视频的宽度和高度
    // - 用于音频的 nb_samples 和 channel_layout
    //
    // 所以需要先设置frame->format, frame->nb_samples , frame->channel_layout
    //
    av_frame_get_buffer_Ret = av_frame_get_buffer(frame, 0);
    CHECK_IF_ERROR_BUF_END(av_frame_get_buffer_Ret < 0, "av_frame_get_buffer");


    // 编码
    // 源文件 ==> (AVFrame)输入缓冲区 ==> 编码器 ==> (AVPacket)输出缓冲区 ==> 输出文件
    while( (readFile_Ret = inFile.read((char *)frame->data[0], frame->linesize[0])) > 0 ) {


        if (readFile_Ret < frame->linesize[0] ) {
            int bytes = av_get_bytes_per_sample((AVSampleFormat) frame->format); //每个样本大小
            int ch = av_get_channel_layout_nb_channels(frame->channel_layout); // 通道数
            frame->nb_samples = readFile_Ret / (bytes * ch); // 样本数量 /  每个样本的总大小

        } else {
            // 编码
            encode_ret = encode(codecCtx, frame, pkt, outFile);
            qDebug() << "encode_ret" << encode_ret;
            CHECK_IF_ERROR_BUF_END(encode_ret < 0, "encode");
        }

    }

    // 在读取最后一次， 冲刷缓冲区
    encode(codecCtx, nullptr, pkt, outFile);

end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codecCtx);
    qDebug() << "AACEncodeThread end ";
}



