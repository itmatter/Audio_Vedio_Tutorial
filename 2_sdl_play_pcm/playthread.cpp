#include "playthread.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <SDL2/SDL.h>
}

#ifdef Q_OS_WIN
    #define FILEPATH "G:/Resource/"
    #define FILENAME "record_to_pcm.pcm"
    #define FORMAT AUDIO_S16SYS

    #define SAMPLE_RATE 44100
    #define SAMPLE_BIT_DEPTH 32
    #define CHANNELS 2
    #define SAMPLES 4096
    #define SAMPLE_PER_BYTES ((SAMPLE_BIT_DEPTH * CHANNELS) / 8)
    #define BUFFER_SIZE (SAMPLES * SAMPLE_PER_BYTES)

#else
    #define FILEPATH "/Users/liliguang/Desktop/"
    #define FILENAME "record_to_pcm.pcm"
    #define FORMAT AUDIO_F32SYS

    #define SAMPLE_RATE 44100       //采样率
    #define SAMPLE_BIT_DEPTH 32     //位深度
    #define CHANNELS 2              //声道数
    #define SAMPLES 4096            //音频缓存区的样本大小
    #define SAMPLE_PER_BYTES ((SAMPLE_BIT_DEPTH * CHANNELS) / 8)    //每个样本占多少字节 (1byte = 8bit)
    #define BUFFER_SIZE (SAMPLES * SAMPLE_PER_BYTES)                //文件缓冲区的大小 (样本大小 *  每个样本占多少字节)
#endif




Playthread::Playthread(QObject *parent) : QThread(parent) {
    // 创建信号连接
    connect(this, &Playthread::finished,
            this, &Playthread::deleteLater);
}

Playthread::~Playthread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "Playthread 析构（内存被回收）";
}


int bufferLen = 0;
Uint8 *bufferData = nullptr;

// 等待音频设备回调(会回调多次)
void pull_audio_data (void *userdata,
                      // 需要往stream中填充PCM数据
                      Uint8 *stream,
                      // 希望填充的大小(samples * format * channels / 8)
                      int len) {
    qDebug() << "pull_audio_data userdata : " << userdata;
    qDebug() << "pull_audio_data stream ： " << stream;
    qDebug() << "pull_audio_data len ： " << len;
    if (stream == nullptr) {
        return;
    }
    SDL_memset(stream, 0, len);
    // 数据还没有准备好
    if(bufferLen <= 0) {
        return;
    }
    len = len > bufferLen ?  bufferLen : len;
    SDL_MixAudio(stream, bufferData, len, SDL_MIX_MAXVOLUME);
    bufferData += len;
    bufferLen -= len;
}



/**
 *网上可以搜索到的出现这种问题的原因如下几种：
 * ①程序中出现对空指针的操作；
 * ②数组越界；
 * ③debug模式下使用了release版本的库或者release模式下使用了debug版本的库；
 * ④程序运行所需要的库没有加载进来（一般是dll），找到所需的动态库放到正确位置。
 */
void Playthread::run() {
    if(SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error : " << SDL_GetError();
        return;
    }
    SDL_AudioSpec audioSpec;
    audioSpec.freq = SAMPLE_RATE;
    audioSpec.format = FORMAT;
    audioSpec.channels = CHANNELS;
    audioSpec.samples = SAMPLES;
    audioSpec.callback = pull_audio_data;
    audioSpec.userdata = nullptr;
    if(SDL_OpenAudio(&audioSpec, nullptr)) {
        qDebug() << "SDL_OpenAudio err" << SDL_GetError();
        SDL_Quit();
        return;
    }
    // 打开文件
    QString filename = FILEPATH;
    filename += FILENAME;
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open fail : " << filename;
        SDL_CloseAudio();
        SDL_Quit();
        return;
    }
    // 开始播放(0是取消暂停)
    SDL_PauseAudio(0);
    // 存放从文件中读取的数据
    Uint8 data[BUFFER_SIZE];
    while(!isInterruptionRequested()) {
        if (bufferLen > 0) {
            continue;
        }
        bufferLen = file.read((char *)data, BUFFER_SIZE);
        //文件数据读取完毕
        if (bufferLen <= 0) {
            break;
        }
        bufferData = data;
    }
    // 关闭文件
    file.close();
    // 关闭设备
    SDL_CloseAudio();
    // 清除所有子系统
    SDL_Quit();
}

void Playthread::setStop(bool stop) {
    _stop = stop;
}
