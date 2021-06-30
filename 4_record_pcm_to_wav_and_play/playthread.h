#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H


#include <QThread>

class PlayThread : public QThread {
    Q_OBJECT
private:
    void run();
    bool _stop = false;

public:
    explicit PlayThread(QObject *parent = nullptr);
    ~PlayThread();
    void setStop(bool stop);
signals:

};

#endif // AUDIOTHREAD_H
