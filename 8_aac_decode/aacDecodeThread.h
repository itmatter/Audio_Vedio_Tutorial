#ifndef AACDECODETHREAD_H
#define AACDECODETHREAD_H

#include <QThread>
class AACDecodeThread: public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit AACDecodeThread(QObject *parent = nullptr);
    ~AACDecodeThread();
};

#endif // AACDECODETHREAD_H
