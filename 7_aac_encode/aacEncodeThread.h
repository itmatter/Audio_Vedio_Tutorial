#ifndef AACENCODETHREAD_H
#define AACENCODETHREAD_H

#include <QThread>
class AACEncodeThread: public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit AACEncodeThread(QObject *parent = nullptr);
    ~AACEncodeThread();
};

#endif // AACENCODETHREAD_H
