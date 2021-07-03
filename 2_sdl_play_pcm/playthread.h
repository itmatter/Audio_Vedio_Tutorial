#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
class Playthread : public QThread {
    Q_OBJECT
private:
    void run();
    bool _stop = false;

public:
    explicit Playthread(QObject *parent = nullptr);
    ~Playthread();
    void setStop(bool stop);
};

#endif // PLAYTHREAD_H
