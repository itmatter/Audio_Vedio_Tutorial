#include "h264DecodeThread.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
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
    #define IN_H264_FILEPATH "G:/Resource/in.h264"
    #define OUT_H264_FILEPATH "G:/Resource/out.yuv"
#else
    #define IN_H264_FILEPATH "/Users/liliguang/Desktop/dstYuv.h264"
    #define OUT_H264_FILEPATH "/Users/liliguang/Desktop/h264_out.yuv"
#endif

#define VIDEO_INBUF_SIZE 4096

H264DecodeThread::H264DecodeThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &H264DecodeThread::finished,
            this, &H264DecodeThread::deleteLater);
}

H264DecodeThread::~H264DecodeThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

static int frameIdx = 0;

// 音频解码
// 返回负数：中途出现了错误
// 返回0：解码操作正常完成
static int decode(AVCodecContext *ctx,
                  AVFrame *frame,
                  AVPacket *pkt,
                  QFile &outFile) {

    // 发送数据到解码 , sent_ret = 0 为sucesss
    int ret = avcodec_send_packet(ctx, pkt);

    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_packet error" << errbuf;
        return ret;
    }

    while (1) {
        // 从解码器中获取到数据到frame
        ret = avcodec_receive_frame(ctx, frame);
        qDebug() << "avcodec_receive_frame : " << ret ;



        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF ) {
            return ret;
        } else  if (ret < 0) {
            qDebug() << "ret < 0" << ret ;
            return ret;
        }

        qDebug() << "解码出第" << ++frameIdx << "帧";
        // 将解码后的数据写入文件
        // 写入Y平面
        outFile.write((char *) frame->data[0],
                      frame->linesize[0] * ctx->height);
        // 写入U平面
        outFile.write((char *) frame->data[1],
                      frame->linesize[1] * ctx->height >> 1);
        // 写入V平面
        outFile.write((char *) frame->data[2],
                      frame->linesize[2] * ctx->height >> 1);

        // 成功从编码器拿到编码后的数据
        // 将编码后的数据写入文件
//        outFile.write((char *) frame->data[0], frame->linesize[0]);
//        return 0;
    }
}




void H264DecodeThread::run() {
    qDebug() << "H264DecodeThread run ";

    // 解码器
    const AVCodec *codec = nullptr;
    // 解码器上下文
    AVCodecContext *codecCtx = nullptr;
    // Parser上下文
    AVCodecParserContext *codecParserCtx = nullptr;
    // 源文件数据源存储结构指针
    AVFrame *frame = nullptr;
    // 编码文件数据源存储结构指针
    AVPacket *pkt = nullptr;

    int avcodec_open2_Ret;

    // 输入输出文件
    const char *infilename;
    const char *outfilename;

    infilename = IN_H264_FILEPATH;
    outfilename = OUT_H264_FILEPATH;

    QFile inFile(infilename);
    QFile outFile(outfilename);

    int infileOpen_Ret;
    int outfileOpen_Ret;



    // 加上AV_INPUT_BUFFER_PADDING_SIZE是为了防止某些优化过的reader一次性读取过多导致越界.
    char inDataArray[VIDEO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];   // 输入缓冲区
    char *inData = inDataArray;                                          // 指向输入缓冲区指针

    int inLen; // 读取到文件的数据大小
    bool inEnd = false;

    int inParserRet;
    int decode_ret;
    // ============================================================
    // 解码逻辑  源文件 ==> 解析器 ==> (AVPacket)输入缓冲区 ==> 解码器 ==> (AVFrame)输出缓冲区 ==> 输出文件

    // 输入文件
    infileOpen_Ret = inFile.open(QFile::ReadOnly);
    CHECK_IF_ERROR_BUF_END(!infileOpen_Ret, "inFile.open");
    // 输出文件
    outfileOpen_Ret = outFile.open(QFile::WriteOnly);
    CHECK_IF_ERROR_BUF_END(!outfileOpen_Ret, "outFile.open");

    // 解码器
    codec = avcodec_find_decoder_by_name("h264");
    CHECK_IF_ERROR_BUF_END(!codec, "avcodec_find_decoder");

    // 解析器上下文
    codecParserCtx = av_parser_init(codec->id);
    CHECK_IF_ERROR_BUF_END(!codecParserCtx, "av_parser_init");

    // 解码器上下文
    codecCtx = avcodec_alloc_context3(codec);
    CHECK_IF_ERROR_BUF_END(!codecCtx, "avcodec_alloc_context3");

    // 打开解码器
    avcodec_open2_Ret = avcodec_open2(codecCtx, codec, nullptr);
    CHECK_IF_ERROR_BUF_END(avcodec_open2_Ret, "avcodec_open2");

    // 创建输入Packet
    pkt = av_packet_alloc();
    CHECK_IF_ERROR_BUF_END(!pkt, "av_packet_alloc");

    // 创建输出rame
    frame = av_frame_alloc();
    CHECK_IF_ERROR_BUF_END(!frame, "av_frame_alloc");



    do {
        // 只要还没有到文件结尾, 每次都读取一次文件
        inLen = inFile.read(inDataArray, VIDEO_INBUF_SIZE);
        inEnd = !inLen;
        // 每次将inData的位置重置为buffer缓冲区的首位置
        inData = inDataArray;

        // 如果不是文件结尾
        while (inLen > 0 || inEnd) {

            // 传给parser
            inParserRet = av_parser_parse2(codecParserCtx,
                                           codecCtx,
                                           &pkt->data,
                                           &pkt->size,
                                           (uint8_t *)inData,
                                           inLen,
                                           AV_NOPTS_VALUE,
                                           AV_NOPTS_VALUE,
                                           0);

            // 如果经过parser 处理返回的内容大于0, 那么就是解码成功
            CHECK_IF_ERROR_BUF_END(inParserRet < 0, "av_parser_parse2");


            inData += inParserRet;
            inLen  -= inParserRet;

            qDebug() << "inLen : " << inLen << "inEnd : " << inEnd << " pkt->size : " << pkt->size << "inParserRet : " << inParserRet;

            if (pkt->size) {
                decode_ret = decode(codecCtx, frame,  pkt, outFile);
                CHECK_IF_ERROR_BUF_END( (decode_ret != AVERROR(EAGAIN) && decode_ret != AVERROR_EOF && decode_ret < 0) , "decode");
            }


            // 如果到了文件尾部
            if (inEnd) break;


        }
        qDebug() << " " ;
        qDebug() << "下一次读取" ;

    } while (!inEnd);



    // 冲刷最后一次缓冲区
    decode_ret = decode(codecCtx, frame, nullptr, outFile);
    qDebug() << "H264DecodeThread Last Decode " << decode_ret;
    CHECK_IF_ERROR_BUF_END(decode_ret < 0, "decode");



end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    av_frame_free(&frame);
    av_packet_free(&pkt);

    avcodec_free_context(&codecCtx);
    av_parser_close(codecParserCtx);
    qDebug() << "H264DecodeThread end ";
}



