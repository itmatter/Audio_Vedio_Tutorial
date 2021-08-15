#include "playthread.h"
#include <QDebug>
#include <QFile>


extern "C" {
#include <SDL2/SDL.h>
}


#define FILENAME "G:/BigBuckBunny_CIF_24fps.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 352
#define IMG_H 288
#define FRAME_RATE 24


// 出错了就执行goto end
#define END(judge, func) \
    if (judge) { \
        qDebug() << #func << "Error" << SDL_GetError(); \
        goto end; \
    }


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


void Playthread::run() {
    if(SDL_Init(SDL_INIT_VIDEO)) {
        qDebug() << "SDL_Init error : " << SDL_GetError();
        return;
    }

    // 创建一个SDL Window窗口
    SDL_Window *window = nullptr;
    // 像素数据
    SDL_Surface *surface = nullptr;
    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture *texture = nullptr;
    // 渲染上下文
    SDL_Renderer *renderer = nullptr;

    int renderUpdateRet;
    int renderCopyRet;
    int renderDrawColor;

    int openRet;
    int readRet = 0;

    //YUV文件输入
    QFile infile(FILENAME);

    // 一帧的数据大小 yuv420p每个像素1.5个字节
    int imgSize = IMG_W * IMG_H * 1.5;

    // 创建一个buffer缓冲区
    char data[imgSize];
    window = SDL_CreateWindow("SDL播放YUV",
                              200, 200,
                              IMG_W * 2, IMG_H * 2,
                              SDL_WINDOW_SHOWN);
    END(!window, SDL_CreateWindow);

    renderer = SDL_CreateRenderer(window, -1,  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { // 说明开启硬件加速失败
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    END(!renderer, SDL_CreateRenderer);


    // 核心处理的代码，将一帧的数据渲染到屏幕上，现在这里是播放yuv,输入的帧率，1s多少帧的填充刷新
    // 我们现在是读取文件的内容传入


    // 贴图纹理
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, IMG_W, IMG_H);
    END(!texture, SDL_CreateTextureFromSurface);

    // YUV420p ==> yyyyuu
    qDebug() << "texture" << texture;

    // 打开文件
    openRet = infile.open(QFile::ReadOnly);
    END(!openRet, infile.open);
    qDebug() << "openRet" << openRet;


    while((readRet = infile.read(data, imgSize)) != 0) {
        qDebug() << "readRet" << readRet;

        // 将YUV数据更新到render中
        renderUpdateRet = SDL_UpdateTexture(texture, nullptr, data, IMG_W);
        END(renderUpdateRet, SDL_RenderCopy);

        renderDrawColor = SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        END(renderDrawColor, SDL_SetRenderDrawColor);

        // 拷贝纹理数据到渲染目标（默认是window）
        renderCopyRet = SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        END(renderCopyRet, SDL_RenderCopy);

        // 将此前的所有需要渲染的内容更新到屏幕上
        SDL_RenderPresent(renderer);
        SDL_Delay(40);

    }

end:


    SDL_DestroyWindow(window);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    // 清除所有子系统
    SDL_Quit();
}

//void Playthread:: setSDLWindow(SDL_Window *window) {
//    _window = window;
//}


void Playthread::setStop(bool stop) {
    _stop = stop;
}
