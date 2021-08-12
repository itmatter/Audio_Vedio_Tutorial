#include "playthread.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <SDL2/SDL.h>
}


#define FILENAME "G:/Resource/out_yuv420p.yuv"
#define IMG_W 512
#define IMG_H 512


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
    int openRet;

    //YUV文件输入
    QFile infile(FILENAME);

    window = SDL_CreateWindow("SDL播放BMP",
                              100, 100,
                              600, 600,
                              SDL_WINDOW_SHOWN);
    END(!window, SDL_CreateWindow);


    renderer = SDL_CreateRenderer(window, -1,  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { // 说明开启硬件加速失败
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    END(!renderer, SDL_CreateRenderer);


    // 贴图纹理
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, IMG_W, IMG_H);
    END(!texture, SDL_CreateTextureFromSurface);

    // YUV420p ==> yyyyuu
    qDebug() << "texture" << texture;

    // 打开文件
    openRet = infile.open(QFile::ReadOnly);
    END(!openRet, infile.open);
    qDebug() << "openRet" << openRet;

    // 将YUV数据更新到render中
    renderUpdateRet = SDL_UpdateTexture(texture, nullptr, infile.readAll().data(), IMG_W);
    END(renderUpdateRet, SDL_RenderCopy);

    // 拷贝纹理数据到渲染目标（默认是window）
    renderCopyRet = SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    END(renderCopyRet, SDL_RenderCopy);

    // 将此前的所有需要渲染的内容更新到屏幕上
    SDL_RenderPresent(renderer);

    // 延迟3秒退出
    SDL_Delay(3000);

end:


    SDL_DestroyWindow(window);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    // 清除所有子系统
    SDL_Quit();
}

void Playthread::setStop(bool stop) {
    _stop = stop;
}
