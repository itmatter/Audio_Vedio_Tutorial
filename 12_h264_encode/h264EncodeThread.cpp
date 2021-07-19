#include "h264EncodeThread.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
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

#else
    #define IN_PCM_FILEPATH "/Users/liliguang/Desktop/out.yuv"
    #define OUT_AAC_FILEPATH "/Users/liliguang/Desktop/out_encode_h264.h264"
    // 样本参数
    #define BIT_RATE 400000
    // 分辨率(必须是2的倍数)
    #define WIDTH 1280
    #define HEIGHT 720
    // 每秒帧数
    #define TIME_BASE 30
    #define FRAMERATE 30
    /* 在将帧传递给编码器之前，每十帧发出一个帧内检查帧 pict_type，
    * 如果 frame->pict_type 是 AV_PICTURE_TYPE_I，
    * 则忽略 gop_size 并且编码器的输出将始终为 I 帧，而与 gop_size 无关
    */
    #define GOP_SIZE 10
    #define MAX_B_FRAMES 1
    #define PIX_FMT AV_PIX_FMT_YUV420P
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

    int ptsIndex = 1;

    int frameSize =av_image_get_buffer_size(PIX_FMT,WIDTH,HEIGHT,1);


    // 编码器
    // codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    codec = avcodec_find_encoder_by_name("libx264"); // 用查找编码器的名称的方式。 默认的可能找到的不一样
    CHECK_IF_ERROR_BUF_END(!codec, "avcodec_find_encoder");

    // 创建编码器上下文
    codecCtx = avcodec_alloc_context3(codec);
    CHECK_IF_ERROR_BUF_END(!codecCtx, "avcodec_alloc_context3");

    // 设置编码器上下文参数


    codecCtx->bit_rate = BIT_RATE;
    codecCtx->width = WIDTH;
    codecCtx->height = HEIGHT;
    codecCtx->time_base = (AVRational){1, TIME_BASE};
    codecCtx->framerate = (AVRational){FRAMERATE, 1};
    codecCtx->gop_size = GOP_SIZE;
    codecCtx->max_b_frames = MAX_B_FRAMES;
    codecCtx->pix_fmt = PIX_FMT;

    // 打开编码器
    avcodec_open2_Ret = avcodec_open2(codecCtx, codec, nullptr);
    CHECK_IF_ERROR_BUF_END(avcodec_open2_Ret, "avcodec_open2");
    qDebug() << "avcodec_open2_Ret" << avcodec_open2_Ret;


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

    // 设置frame必要信息,
    frame->format = codecCtx->pix_fmt;
    frame->width  = codecCtx->width;
    frame->height = codecCtx->height;


    // 为音频或视频数据分配新的缓冲区。
    // 在调用此函数之前，必须在框架上设置以下字段：
    // - format（视频的像素格式，音频的样本格式）
    // - 视频的width和height
    // - 用于音频的 nb_samples 和 channel_layout
    //
    // 所以需要先设置frame->format, frame->width , frame->height
    //
    av_frame_get_buffer_Ret = av_frame_get_buffer(frame, 0);
    CHECK_IF_ERROR_BUF_END(av_frame_get_buffer_Ret < 0, "av_frame_get_buffer");


    // 编码
    // 源文件 ==> (AVFrame)输入缓冲区 ==> 编码器 ==> (AVPacket)输出缓冲区 ==> 输出文件

    // 编码, 以每一帧来看

    // 关于frame->linesize[0] :  For video, size in bytes of each picture line.
    while( (readFile_Ret = inFile.read((char *)frame->data[0], frameSize )) > 0 ) {

        frame->pts = ptsIndex++;
        encode_ret = encode(codecCtx, frame, pkt, outFile);
        qDebug() << "readFile_Ret" << readFile_Ret << "encode_ret" << encode_ret;
        CHECK_IF_ERROR_BUF_END(encode_ret < 0, "encode");

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



