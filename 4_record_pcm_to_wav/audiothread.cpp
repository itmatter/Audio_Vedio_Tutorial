#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

#include "wavhander.h"
extern "C" {
    // 设备
#include <libavdevice/avdevice.h>
    // 格式
#include <libavformat/avformat.h>
    // 工具（比如错误处理）
#include <libavutil/avutil.h>
}

#ifdef Q_OS_WIN
    //# 查看dshow支持的设备
    //ffmpeg -f dshow -list_devices true -i dummy
    #define FMT_NAME "dshow"
    //    #define DEVICE_NAME "audio=耳机 (靓模袭地球 Hands-Free AG Audio)"
    #define DEVICE_NAME "audio=麦克风 (Realtek High Definition Audio)"
    #define FILEPATH "G:/Resource/"
    #define FILENAME "record_to_pcm.pcm"
    #define WAVFILEPATH "G:/Resource/"
    #define WAVFILENAME "pcm_to_wav.wav"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILEPATH "/Users/liliguang/Desktop/"
    #define FILENAME "record_to_pcm.pcm"
    #define WAVFILEPATH "/Users/liliguang/Desktop/"
    #define WAVFILENAME "pcm_to_wav.wav"
#endif



AudioThread::AudioThread(QObject *parent) : QThread(parent) {
//    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}


AudioThread::~AudioThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}


void showSpec(AVFormatContext *ctx) {
    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取音频参数
    AVCodecParameters *params = stream->codecpar;
    // 声道数
    qDebug() << "params->channels" << params->channels;
    // 采样率
    qDebug() << "params->sample_rate" << params->sample_rate;
    // 采样格式
    qDebug() << "params->format" << params->format;
    qDebug() << "params->channel_layout" << params->channel_layout;
    qDebug() << "params->codec_id" << av_get_bits_per_sample(params->codec_id);
    // 每一个样本的一个声道占用多少个字节
    qDebug() << av_get_bytes_per_sample((AVSampleFormat) params->format);
}


// 需要注意的是, Mac系统下需要用Debug模式运行, 会弹出访问权限.
void AudioThread::run() {
    qDebug() << this << "开始执行----------";
    // 1-----获取输入格式对象
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }
    qDebug() << "fmt : " << fmt;
    // 2-----格式上下文（将来可以利用上下文操作设备）
    AVFormatContext *ctx = nullptr;
    // 打开设备


    qDebug() << "DEVICE_NAME : " << DEVICE_NAME;

    int ret = avformat_open_input(&ctx, ":0", fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "打开设备失败" << errbuf;
        return;
    }
    qDebug() << "ret : " << ret;
    // 打印一下录音设备的参数信息
    showSpec(ctx);
    // 文件名
    // 3-----打开文件
    QString filename = FILEPATH;
    filename += FILENAME;
    QFile file(filename);
    // WriteOnly：只写模式。如果文件不存在，就创建文件；如果文件存在，就会清空文件内容
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;
        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }




    // 4----采集数据
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

    // 当不再需要数据包时 The packet must be freed with av_packet_unref() when  it is no longer needed.
    av_packet_unref(&pkt);

    // 释放资源 关闭文件
    file.close();
    // 5----录制WAV文件
    AVStream *stream = ctx->streams[0];
    AVCodecParameters *param = stream->codecpar;
    //调用函数
    WAVHeader header;
    header.numChannel = param->channels;
    header.sampleRate = param->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(param->codec_id);
    QString wavFileName = WAVFILEPATH;
    wavFileName += WAVFILENAME;
    WAVhander::pcm_To_wav(header, filename.toLatin1().data(), wavFileName.toLatin1().data() );
    // 6----关闭设备
    avformat_close_input(&ctx);
    qDebug() << this << "正常结束----------";
}

void AudioThread::setStop(bool stop) {
    _stop = stop;
}





