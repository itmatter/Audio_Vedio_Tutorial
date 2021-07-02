#ifndef WAVHANDER_H
#define WAVHANDER_H

#include <stdint.h>

/*
 * 复习 :
 * short占据的内存大小是2 个byte；
 * int占据的内存大小是4 个byte；
 * long占据的内存大小是4 个byte；
 * float占据的内存大小是4 个byte；
 * double占据的内存大小是8 个byte；
 * char占据的内存大小是1 个byte。
 *
 *
 * uint8_t 无符号1个字节的整型
 * uint16_t 无符号2个字节的整型
 * uint32_t 无符号4个字节的整型
 * uint64_t 无符号8个字节的整型

 *
 */


#define AUDIO_FORMAT_PCM 1
#define AUDIO_FORMAT_FLOAT 3

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
