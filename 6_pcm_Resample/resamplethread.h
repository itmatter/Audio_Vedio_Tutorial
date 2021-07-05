#ifndef RESAMPLETHREAD_H
#define RESAMPLETHREAD_H

#include <QThread>
class ResampleThread: public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit ResampleThread(QObject *parent = nullptr);
    ~ResampleThread();



};

#endif // RESAMPLETHREAD_H
