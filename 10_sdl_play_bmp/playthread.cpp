#include "playthread.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <SDL2/SDL.h>
}


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
    //
    int renderCopyRet;

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

    // 3.处理像素数据
    surface = SDL_LoadBMP("G:/Resource/in.bmp");
    END(!surface, SDL_LoadBMP);

    //  4.贴图纹理
    texture = SDL_CreateTextureFromSurface( renderer, surface);
    END(!texture, SDL_CreateTextureFromSurface);

    // 复制纹理到渲染目标上
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
