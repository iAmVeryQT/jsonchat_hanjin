#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void recvEchoMsg();

    void on_connBtn_clicked();

    void on_chatText_returnPressed();

    void on_sendFile_clicked();

    void on_getItem_released();

    void onDownloadBtn();

private:
    Ui::MainWindow *ui;
    QTcpSocket jsonServer;
    QMap<QString, QString> MSG_TYPE;
    QMap<QString, QString> MSG_SUB_TYPE;
};

#endif // MAINWINDOW_H
