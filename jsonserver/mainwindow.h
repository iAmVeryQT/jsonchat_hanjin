#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chatserver.h"
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void ServerOn();
    void ServerOff();

private:
    void closeEvent(QCloseEvent* e) Q_DECL_OVERRIDE; // 오버라이드 명시

private:
    Ui::MainWindow *ui;
    ChatServer mServer;
};

#endif // MAINWINDOW_H
