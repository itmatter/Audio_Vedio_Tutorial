#ifndef REFORMATTHREAD_H
#define REFORMATTHREAD_H

#include <QThread>
class ReformatThread: public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit ReformatThread(QObject *parent = nullptr);
    ~ReformatThread();
};

#endif // REFORMATTHREAD_H
