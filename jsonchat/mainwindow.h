#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFile>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void processStreamMsg(const char* data);

    void captureJSON(const QJsonDocument jsonDoc);

private slots:

    void receiveMsgFromServer();

    void on_connBtn_clicked();

    void on_chatText_returnPressed();

    void on_sendFile_clicked();

    void on_getItem_released();

    void onClickDNBtn();

private:
    Ui::MainWindow *ui;
    QTcpSocket jsonServer;
    QMap<QString, QString> MSG_TYPE;
    QMap<QString, QString> MSG_SUB_TYPE;
    QList<QFile*> dnIngFileList;
    QString receivedData;
    QString host;
};

#endif // MAINWINDOW_H
