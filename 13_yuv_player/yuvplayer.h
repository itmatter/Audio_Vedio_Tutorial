#ifndef YUVPLAYER_H
#define YUVPLAYER_H


extern "C" {
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
}

#include <QWidget>
#include <QFile>

typedef struct  {
    const char *fileName; //文件路径
    int width;  //分辨率-宽
    int height; //分辨率-高
    int framerate;  //帧率
    SDL_PixelFormatEnum pixelFormat;//像素格式
    int timeInterval; //跟定时器相关
} YuvParams;

class YuvPlayer : public QWidget {
    Q_OBJECT

private:
    // 创建一个SDL Window窗口
    SDL_Window *_window = nullptr;
    // 像素数据
    SDL_Surface *_surface = nullptr;
    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture *_texture = nullptr;
    // 渲染上下文
    SDL_Renderer *_renderer = nullptr;

    // 加载的文件路径
    QFile _infile;

    QTimer *qTimer = nullptr;

    // 是否在播放中， 默认false
    bool _playing = false;
    // 是否暂停播放， 默认false
    bool _pause = false;
    // 是否终止播放， 默认true
    bool _stop = true;


    // yuv参数
    YuvParams _yuvParams;
    void loadYUVData();

public:
    explicit YuvPlayer(QWidget *parent = nullptr);
    ~YuvPlayer();


    void play(YuvParams yuvParam);
    bool isPlaying();

    void pause();
    bool isPause();

    void stop();
    bool isStop();



signals:

};

#endif // YUVPLAYER_H
