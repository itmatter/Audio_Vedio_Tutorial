#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "resamplethread.h"
#include "swsscalethread.h"

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
    void on_reSampleBtn_clicked();

    void on_swsScaleBtn_clicked();

private:
    Ui::MainWindow *ui;
    ResampleThread *_resampleThread = nullptr;
    SwsScaleThread *_swsScaleThread = nullptr;

};
#endif // MAINWINDOW_H
