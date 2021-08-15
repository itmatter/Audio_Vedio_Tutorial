#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "yuvPlayer.h"

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

    void on_playBtn_clicked();

private:
    Ui::MainWindow *ui;
    YUVPlayer *_yuvPlayer = nullptr;

};
#endif // MAINWINDOW_H
