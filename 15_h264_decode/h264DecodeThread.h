#ifndef H264DECODETHREAD_H
#define H264DECODETHREAD_H

#include <QThread>
class H264DecodeThread: public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit H264DecodeThread(QObject *parent = nullptr);
    ~H264DecodeThread();
};

#endif // H264DECODETHREAD_H
