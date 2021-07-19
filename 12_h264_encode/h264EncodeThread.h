#ifndef H264ENCODETHREAD_H
#define H264ENCODETHREAD_H

#include <QThread>
class H264EncodeThread: public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit H264EncodeThread(QObject *parent = nullptr);
    ~H264EncodeThread();
};

#endif // H264ENCODETHREAD_H
