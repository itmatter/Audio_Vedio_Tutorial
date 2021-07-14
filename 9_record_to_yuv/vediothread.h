#ifndef VEDIOTHREAD_H
#define VEDIOTHREAD_H

#include <QThread>

class VedioThread : public QThread {
    Q_OBJECT
private:
    void run();
    bool _stop = false;

public:
    explicit VedioThread(QObject *parent = nullptr);
    ~VedioThread();
    void setStop(bool stop);
signals:

};

#endif // VEDIOTHREAD_H
