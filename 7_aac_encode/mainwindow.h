#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "aacEncodeThread.h"

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
    void on_aacEncodeBtn_clicked();

private:
    Ui::MainWindow *ui;
    AACEncodeThread *_aacEncodeThread = nullptr;

};
#endif // MAINWINDOW_H
