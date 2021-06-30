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


// Mac系统下使用命令行查看录音设备
// ffmpeg -f avfoundation -list_devices true -i ''
//
// AVFoundation video devices:
// [0] FaceTime HD Camera
// [1] Capture screen 0
// AVFoundation audio devices:
// [0] Built-in Microphone
//
//
// 格式名称，设备名称
#ifdef Q_OS_WIN
    //# 查看dshow支持的设备
    //ffmpeg -f dshow -list_devices true -i dummy
    #define FMT_NAME "dshow"
    //    #define DEVICE_NAME "audio=耳机 (靓模袭地球 Hands-Free AG Audio)"
    #define DEVICE_NAME "audio=麦克风 (Realtek High Definition Audio)"
    #define FILEPATH "G:/Resource/"
    #define FILENAME "record_pcm_to_wav.wav"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILEPATH "/Users/lumi/Desktop/"
    #define FILENAME "record_to_pcm.pcm"
#endif



/*
 * 通过ffmpeg录制PCM, 录制完毕之后, 通过命令行播放,
 * ffplay -ar 44100 -ac 2 -f f32le xxx.pcm
 *
 * 可以通过ffmpeg命令行录制音频得知当前的音频录制格式,
 * ffmpeg -f avfoundation -i :0 record.wav
 *
 * Input #0, avfoundation, from ':0':
 * Duration: N/A, start: 91244.807710, bitrate: 2822 kb/s
 * Stream #0:0: Audio: pcm_f32le, 44100 Hz, stereo, flt, 2822 kb/s
 * Stream mapping:
 * Stream #0:0 -> #0:0 (pcm_f32le (native) -> pcm_s16le (native))
 * Press [q] to stop, [?] for help
 *
 * 从输入流中得知 原始输入信息 : 44100(采样率) stereo(双声道) pcm_f32le
 *
 *
 *
 * 如果是Win
 * 录制 ：ffmpeg -f dshow -i "audio=耳机 (靓模袭地球 Hands-Free AG Audio)" out.wav
 * 得到录制格式 s16le
 * 播放 ：ffplay -ar 44100 -ac 2 -f s16le .\Resourcerecord_to_pcm.pcm
 */
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


void AudioThread::run() {
    qDebug() << this << "开始执行----------";
    // 获取输入格式对象
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }
    qDebug() << "fmt : " << fmt;
    // 格式上下文（将来可以利用上下文操作设备）
    AVFormatContext *ctx = nullptr;
    // 打开设备
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
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
    QString filename = FILEPATH;
//    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
//    filename += ".pcm";
    filename += FILENAME;
    QFile file(filename);
    // 打开文件
    // WriteOnly：只写模式。如果文件不存在，就创建文件；如果文件存在，就会清空文件内容
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;
        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }
    // 数据包
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
    // 释放资源
    // 关闭文件
    file.close();
    // 录制WAV文件
    AVStream *stream = ctx->streams[0];
    AVCodecParameters *param = stream->codecpar;
    //调用函数
    WAVHeader header;
    header.numChannel = param->channels;
    header.sampleRate = param->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(param->codec_id);
    WAVhander::pcm_To_wav(header,
                          (char *)"/Users/lumi/Desktop/record_to_pcm.pcm",
                          (char *)"/Users/lumi/Desktop/pcm_To_wav.wav" );
    // 关闭设备
    avformat_close_input(&ctx);
    qDebug() << this << "正常结束----------";
}

void AudioThread::setStop(bool stop) {
    _stop = stop;
}



