#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
class Playthread : public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit Playthread(QObject *parent = nullptr);
    ~Playthread();


};

#endif // PLAYTHREAD_H
