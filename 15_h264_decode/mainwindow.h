#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "h264DecodeThread.h"

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
    void on_decodeBtn_clicked();

private:
    Ui::MainWindow *ui;
    H264DecodeThread *_h264DecodeThread = nullptr;

};
#endif // MAINWINDOW_H
