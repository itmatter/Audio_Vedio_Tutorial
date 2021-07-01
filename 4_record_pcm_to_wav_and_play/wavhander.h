#ifndef WAVHANDER_H
#define WAVHANDER_H

#include <stdint.h>

#define AUDIO_FORMAT_PCM 1
#define AUDIO_FORMAT_FLOAT 3

// 注意 :
// 注意 :
// 注意 :
// 注意 : 这里转成WAV 对应的编码格式是 Stream #0:0: Audio: pcm_s16le ([1][0][0][0] / 0x0001), 44100 Hz, 2 channels, s16, 1411 kb/s
// 注意 : 也就是如果电脑本身是 f32le的编码格式, 那么就播放不了, 这里就需要重采样为f32le


#ifdef Q_OS_WIN
    #define AUDIO_FORMAT AUDIO_FORMAT_PCM
#else
    #define AUDIO_FORMAT AUDIO_FORMAT_FLOAT
#endif



// 创建一个结构体
typedef struct {
    // RIFF Chunk
    uint8_t riffChunkID[4] = {'R', 'I', 'F', 'F'};
    uint32_t riffChunkSize;

    // DATA
    uint8_t format[4] = {'W', 'A', 'V', 'E'};

    // FMT Chunk
    uint8_t fmtChunkID[4] = {'f', 'm', 't', ' '};
    uint32_t fmtChunkSize = 16;

    //编码格式(音频编码)
    uint16_t audioFormat = AUDIO_FORMAT;
    //声道数
    uint16_t numChannel;
    //采样率
    uint32_t sampleRate;
    //字节率
    uint32_t byteRate;
    //一个样本的字节数
    uint16_t blockAlign;
    // 位深度
    uint16_t bitsPerSample;


    // DATA Chunk
    uint8_t dataChunkID[4] = {'d', 'a', 't', 'a'};
    uint32_t dataChunkSize;

} WAVHeader;




class WAVhander {

public:
    static void pcm_To_wav(WAVHeader &header,
                           char *pcmFile,
                           char *wavFile);
};

#endif // WAVHANDER_H
