#include "h264encodethread.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
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
    #define IN_YUV_FILEPATH "G:/BigBuckBunny_CIF_24fps.yuv"
    #define OUT_H264_FILEPATH "G:/BigBuckBunny_CIF_24fps_h264.h264"
    #define YUV_VIDEO_SIZE_WIDTH 352
    #define YUV_VIDEO_SIZE_HEIGH 288
#else
    #define IN_YUV_FILEPATH "/Users/liliguang/Desktop/record_to_pcm.pcm"
    #define OUT_H264_FILEPATH "/Users/liliguang/Desktop/pcm_to_aac.aac"
    #define YUV_VIDEO_SIZE_WIDTH 352
    #define YUV_VIDEO_SIZE_HEIGH 288
#endif



H264EncodeThread::H264EncodeThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &H264EncodeThread::finished,
            this, &H264EncodeThread::deleteLater);
}

H264EncodeThread::~H264EncodeThread() {
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
static int check_pixel_fmt(const AVCodec *codec,
                           enum AVPixelFormat pix_fmt) {
    const enum AVPixelFormat *p = codec->pix_fmts;
    while (*p != AV_PIX_FMT_NONE) {
        if (*p == pix_fmt) {
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




void H264EncodeThread::run() {
    // H264编码的思路和Acc编码的思路差不多
    // 读取文件内容 -> buffer缓冲区 -> 一帧数据 -> 核心函数编码 -> 输出缓冲区 -> 输出文件

    qDebug() << "H264EncodeThread run ";

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

    int check_pixel_fmt_Ret;
    int avcodec_open2_Ret;
    int av_image_alloc_ret;

    int infileOpen_Ret;
    int outfileOpen_Ret;

    int readFile_Ret;
    int encode_ret;

    int ptsIndex = 0;
    int imageSize = YUV_VIDEO_SIZE_HEIGH * YUV_VIDEO_SIZE_WIDTH * 1.5;



    infilename = IN_YUV_FILEPATH;
    outfilename = OUT_H264_FILEPATH;

    QFile inFile(infilename);
    QFile outFile(outfilename);


    // 编码器
    codec = avcodec_find_encoder_by_name("libx264"); // 用查找编码器的名称的方式。 默认的可能找到的不一样
    CHECK_IF_ERROR_BUF_END(!codec, "avcodec_find_encoder");

    // 创建编码器上下文
    codecCtx = avcodec_alloc_context3(codec);
    CHECK_IF_ERROR_BUF_END(!codecCtx, "avcodec_alloc_context3");

    // 设置编码器上下文对应信息
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    codecCtx->width = YUV_VIDEO_SIZE_WIDTH;
    codecCtx->height = YUV_VIDEO_SIZE_HEIGH;
    // 这里要计算24 pfs
    // fps = codecCtx->time_base.den / codecCtx->time_base.num
    // 24 = 24 / 1
    codecCtx->time_base.num = 1;       //分子
    codecCtx->time_base.den = 24;         //分母


    // 检查编码器支持的样本格式
    check_pixel_fmt_Ret = check_pixel_fmt(codec, codecCtx->pix_fmt);
    CHECK_IF_ERROR_BUF_END(!check_pixel_fmt_Ret, "check_sample_fmt");

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
    frame->format = codecCtx->pix_fmt;//像素格式
    frame->width = codecCtx->width;//分辨率宽
    frame->height = codecCtx->height; //分辨率高

    // 为音频或视频数据分配新的缓冲区。
    // 在调用此函数之前，必须在框架上设置以下字段：
    // - 格式（视频的像素格式，音频的样本格式）
    // - 视频的宽度和高度
    // - 用于音频的 nb_samples 和 channel_layout
    av_image_alloc_ret = av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, (AVPixelFormat)frame->format, 16);
    CHECK_IF_ERROR_BUF_END(av_image_alloc_ret < 0, "av_image_alloc");

    // 编码
    // 源文件 ==> (AVFrame)输入缓冲区 ==> 编码器 ==> (AVPacket)输出缓冲区 ==> 输出文件
    // 这里应该读取一帧的大小
    while( (readFile_Ret = inFile.read((char *)frame->data[0], imageSize )) > 0 ) {
        frame->pts = ptsIndex++;
//        qDebug() << "readFile_Ret" << readFile_Ret;
//        qDebug() << "frame->linesize[0]" << frame->linesize[0];
//        if (readFile_Ret < frame->linesize[0] ) {
//            qDebug() << "最后一次"  ;
//            qDebug() << "readFile_Ret" << readFile_Ret;
//            qDebug() << "frame->linesize[0]" << frame->linesize[0];
//        } else {
        // 编码
        encode_ret = encode(codecCtx, frame, pkt, outFile);
        CHECK_IF_ERROR_BUF_END(encode_ret < 0, "encode");
//        }

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



