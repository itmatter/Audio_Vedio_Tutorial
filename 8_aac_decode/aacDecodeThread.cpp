#include "aacDecodeThread.h"

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
    #define IN_AAC_FILEPATH "G:/Resource/pcm_to_aac.aac"
    #define OUT_PCM_FILEPATH "G:/Resource/aac_decode_to_pcm.pcm"
#else
    #define IN_AAC_FILEPATH "/Users/liliguang/Desktop/pcm_to_aac.aac"
    #define OUT_PCM_FILEPATH "/Users/liliguang/Desktop/aac_decode_to_pcm.pcm"
#endif



AACDecodeThread::AACDecodeThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AACDecodeThread::finished,
            this, &AACDecodeThread::deleteLater);
}

AACDecodeThread::~AACDecodeThread() {
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



// 音频解码
// 返回负数：中途出现了错误
// 返回0：解码操作正常完成
static int decode(AVCodecContext *ctx,
                  AVFrame *frame,
                  AVPacket *pkt,
                  QFile &outFile) {
    // 发送数据到解码
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    // 不断从编码器中取出编码后的数据
    // while (ret >= 0)
    while (true) {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // 继续读取数据到frame，然后送到编码器
            return 0;
        } else if (ret < 0) { // 其他错误
            return ret;
        }

        // 成功从编码器拿到编码后的数据
        // 将编码后的数据写入文件
        outFile.write((char *) frame->data[0], frame->linesize[0]);
    }
}




void AACDecodeThread::run() {
    qDebug() << "AACEncodeThread run ";

    // 输入输出文件
    const char *infilename;
    const char *outfilename;

    // 解码器
    const AVCodec *codec = nullptr;
    // 解码器上下文
    AVCodecContext *codecCtx = nullptr;

    // FFMPEG解码音视频的一般来讲，都是直接从媒体容器文件（网络码流或者封装文件）中，读取出AVPaket传个解码器。
    // 但一般音视频解码并不是在这样的场景下，而是直接给解码器传送裸码流（AAC、h264等）,
    // 此时我们需要知道每次传给解码器的音视频数据大小，即每帧音频/视频大小。
    // AVCodecParser可通过音视频裸码流解析出每帧的大小等信息。

    //解析器上下文
    AVCodecParserContext *codecParserCtx = nullptr;

    // 源文件数据源存储结构指针
    AVFrame *frame;
    // 编码文件数据源存储结构指针
    AVPacket *pkt;

    int avcodec_open2_Ret;

    int infileOpen_Ret;
    int outfileOpen_Ret;

    infilename = IN_AAC_FILEPATH;
    outfilename = OUT_PCM_FILEPATH;

    QFile inFile(infilename);
    QFile outFile(outfilename);

    // 加上AV_INPUT_BUFFER_PADDING_SIZE是为了防止某些优化过的reader一次性读取过多导致越界.
    int indataSize = 20480;                                         // 缓冲区大小
    char inDataArray[indataSize + AV_INPUT_BUFFER_PADDING_SIZE];    // 输入缓冲区
    char *inData = inDataArray;                                     // 指向输入缓冲区指针
    int inDataLen = 0;

    int inParserRet = 0;

    int decode_ret;
    // ============================================================
    // 解码逻辑  源文件 ==> 解析器 ==> (AVPacket)输入缓冲区 ==> 解码器 ==> (AVFrame)输出缓冲区 ==> 输出文件

    // 输入文件
    infileOpen_Ret = !inFile.open(QFile::ReadOnly);
    CHECK_IF_ERROR_BUF_END(infileOpen_Ret, "inFile.open");
    // 输出文件
    outfileOpen_Ret = !outFile.open(QFile::WriteOnly);
    CHECK_IF_ERROR_BUF_END(outfileOpen_Ret, "outFile.open");

    // 解码器
    codec = avcodec_find_decoder_by_name("libfdk_aac");
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


    // 读取文件到解析器中。
    // 解析完成之后存放到pkt
    // pkt送到解码器
    // 输出

    // inDataLen 每次读取文件的长度
    while ( (inDataLen = inFile.read(inDataArray, indataSize)) > 0) {
        inData = inDataArray;

        while(inDataLen > 0) {
            inParserRet = av_parser_parse2(codecParserCtx,
                                           codecCtx,
                                           &pkt->data,
                                           &pkt->size,
                                           (uint8_t *)inData,
                                           inDataLen,
                                           AV_NOPTS_VALUE,
                                           AV_NOPTS_VALUE,
                                           0);

            CHECK_IF_ERROR_BUF_END(inParserRet < 0, "av_parser_parse2");

            // 指针位置偏移，跳过已经解析的数据
            inData += inParserRet;

            //读取的大小减去已经解析的大小
            inDataLen  -= inParserRet;

            // 编码
            decode_ret = decode(codecCtx, frame, pkt, outFile);
            qDebug() << "inParserRet" << inParserRet << "decode_ret" << decode_ret << "pkt->size " << pkt->size ;
            CHECK_IF_ERROR_BUF_END( pkt->size > 0 && decode_ret < 0, "decode_ret");
        }
    }

    // 编码
    decode_ret = decode(codecCtx, frame, nullptr, outFile);
    qDebug() << "AACEncodeThread Last Decode ";

end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    av_frame_free(&frame);
    av_packet_free(&pkt);

    avcodec_free_context(&codecCtx);
    av_parser_close(codecParserCtx);
    qDebug() << "AACEncodeThread end ";
}



