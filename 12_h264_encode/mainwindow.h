#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "h264EncodeThread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_h264EncodeBtn_clicked();

private:
    Ui::MainWindow *ui;
    H264EncodeThread *_h264EncodeThread = nullptr;

};
#endif // MAINWINDOW_H
