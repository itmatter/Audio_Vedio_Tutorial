#include "wavhander.h"
#include <QFile>
#include <QDebug>


// WAV格式参考微软文档 https://docs.microsoft.com/en-us/previous-versions/windows/desktop/bb280526(v=vs.85)

void WAVhander::pcm_To_wav(WAVHeader &header,
                           char *pcmFileName,
                           char *wavFileName) {

    // 检索 设置格式类型的数据 的最小原子单位（以字节为单位）。
    // blockAlign属性的值必须等于Channels与bitsPerSample的乘积除以8（每字节位数）, 固定公式。
    // 软件必须一次处理多个blockAlign字节数据。 写入设备和从设备读取的数据必须始终从块的开头开始。
    header.blockAlign = header.bitsPerSample * header.numChannel >> 3;

    // 比特率(Byte/s) = 采样率 * blockAlign
    header.byteRate = header.sampleRate * header.blockAlign;


    //打开PCM文件
    QFile pcmFile(pcmFileName);
    if (!pcmFile.open(QFile::ReadOnly)) {
        qDebug() << "PCM文件打开失败" << pcmFileName;
        return;
    };

    //(音频数据大小)文件大小
    header.dataChunkSize = pcmFile.size();

    // 文件Data所有数据大小 + WAVHeader大小 - 固定RIFFID -
    header.riffChunkSize = header.dataChunkSize
                            + sizeof(WAVHeader)
                            - sizeof(header.riffChunkID)
                            - sizeof(header.riffChunkSize);


    //打开WAV文件
    QFile wavFile(wavFileName);
    if (!wavFile.open(QFile::WriteOnly)) {
        qDebug() << "PCM文件打开失败" << wavFileName;
        pcmFile.close();
        return;
    };


    //首先写入头部数据
    wavFile.write((const char *)&header,sizeof (WAVHeader));

    //写入data数据
    char buffer[1024];
    int size;
    while ((size = pcmFile.read(buffer,sizeof(buffer))) > 0) {
        wavFile.write(buffer,size);
    }

    qDebug() << "转换完成";

    pcmFile.close();
    wavFile.close();


}

