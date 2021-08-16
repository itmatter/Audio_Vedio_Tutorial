#ifndef SWSSCALETHREAD_H
#define SWSSCALETHREAD_H

#include <QThread>
class SwsScaleThread: public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit SwsScaleThread(QObject *parent = nullptr);
    ~SwsScaleThread();
};

#endif // SWSSCALETHREAD_H
