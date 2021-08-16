#include "yuvplayer.h"
#include <QDebug>
#include <QTimer>
#include <QDate>

YuvPlayer::YuvPlayer(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");

    if(SDL_Init(SDL_INIT_VIDEO)) {
        qDebug() << "SDL_Init error : " << SDL_GetError();
        return;
    }

    _window = SDL_CreateWindowFrom((void *)this->winId());
    if(!_window) {
        qDebug() << "SDL_CreateWindow error : " << SDL_GetError();
        SDL_DestroyWindow(_window);
        return;
    }

    _renderer = SDL_CreateRenderer(_window, -1,  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) { // 说明开启硬件加速失败
        _renderer = SDL_CreateRenderer(_window, -1, 0);
    }
    if(!_renderer) {
        qDebug() << "SDL_CreateRenderer error : " << SDL_GetError();
        SDL_DestroyWindow(_window);
        SDL_DestroyRenderer(_renderer);
        return;
    }
}

YuvPlayer::~YuvPlayer() {
    SDL_DestroyWindow(_window);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyTexture(_texture);
    SDL_Quit();
}

// 播放
void YuvPlayer::play(YuvParams yuvParam) {
    // 如果当前是播放的过程中， 直接跳过
    if(_playing && !_pause && !_stop) {
        return;
    }

    // 如果当前是暂停的过程中， 恢复播放
    if(!_playing && _pause && !_stop) {
        qTimer->start();
        _playing = true;
        _pause = false;
        return;
    }

    // 如果当前是初次播放， 创建定时器
    _playing = true;
    _pause = false;
    _stop = false;

    _yuvParams.fileName = yuvParam.fileName;
    _yuvParams.width = yuvParam.width;
    _yuvParams.height = yuvParam.height;
    _yuvParams.framerate = yuvParam.framerate;
    _yuvParams.pixelFormat = yuvParam.pixelFormat;
    _yuvParams.timeInterval = (1 * 1000 / _yuvParams.framerate);


//    if (_yuvParams.width > 500) {
//        SDL_SetWindowSize(_window, _yuvParams.width * 0.5 , _yuvParams.height * 0.5 );
//    }


    if (!qTimer) {
        _infile.setFileName(_yuvParams.fileName);
        // 打开文件
        if (!_infile.open(QFile::ReadOnly)) {
            qDebug() << "infile.open error : " << SDL_GetError();
            return;
        }
        _texture = SDL_CreateTexture(_renderer, _yuvParams.pixelFormat, SDL_TEXTUREACCESS_STREAMING, _yuvParams.width, _yuvParams.height);
        SDL_SetTextureScaleMode(_texture, SDL_ScaleModeNearest);
        if (!_texture) {
            qDebug() << "SDL_CreateTexture error : " << SDL_GetError();
            return;
        }

        // 创建定时器
        qTimer = new QTimer();
        qTimer->setTimerType(Qt::PreciseTimer);
        qTimer->start(_yuvParams.timeInterval);

        connect(qTimer, &QTimer::timeout, this, [ = ]() {
            this->loadYUVData();
        });
    }
}

//暂停
void YuvPlayer::pause() {
    if(_pause) {
        return;
    }
    _playing = false;
    _pause = true;
    _stop = false;

    // 暂时定时器
    if(qTimer) {
        qTimer->stop();
    }
}

//终止
void YuvPlayer::stop() {
    if(_stop) {
        return;
    }
    _playing = false;
    _pause = false;
    _stop = true;

    qTimer->stop();
    qTimer = nullptr;

    // 停止关闭文件
    _infile.close();

    if(_renderer) {
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(_renderer);
        SDL_RenderPresent(_renderer);
    }
}


bool YuvPlayer::isPlaying() {
    return _playing;
}
bool YuvPlayer::isPause() {
    return _pause;
}
bool YuvPlayer::isStop() {
    return _stop;
}

// 每隔一段时间就会调用
void YuvPlayer::loadYUVData() {
    qDebug() << "Time : " <<  QTime::currentTime();

    int imgSize = _yuvParams.width * _yuvParams.height * 2;
    if (_yuvParams.pixelFormat == SDL_PIXELFORMAT_IYUV) {
        imgSize = _yuvParams.width * _yuvParams.height * 1.5;
    }
    char data[imgSize];
    // 创建一个定时器， 一秒获取固定帧的数据
    if( _infile.read(data, imgSize) > 0) {
        // 将YUV数据更新到render中
        if(SDL_UpdateTexture(_texture, nullptr, data, _yuvParams.width)) {
            qDebug() << "SDL_RenderCopy error : " << SDL_GetError();
        }
        if(SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE)) {
            qDebug() << "SDL_SetRenderDrawColor error : " << SDL_GetError();
        }
        SDL_Rect rect = {0, 0, _yuvParams.width, _yuvParams.height  };
        if(SDL_RenderCopy(_renderer, _texture, nullptr, &rect)) {
            qDebug() << "SDL_RenderCopy error : " << SDL_GetError();
        }
        // 将此前的所有需要渲染的内容更新到屏幕上
        SDL_RenderPresent(_renderer);
    } else {
        this->stop();
    }
}






