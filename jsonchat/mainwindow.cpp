#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#define TITLE "DEBUG"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // enroll signal to slot
    connect(&jsonServer, SIGNAL(readyRead()), this, SLOT(recvEchoMsg()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_hosturl_textChanged()
{
    QString host = ui->hosturl->toPlainText();

    // remove New Line
    if(host.right(1) == "\n") {
        host = host.left(host.size() - 1);
        // apply UI
        ui->hosturl->setText(host);
        // network conn
        jsonServer.connectToHost(host, 10125);
        // waiting conn for 5 sec
        if(jsonServer.waitForConnected(5000)) {
            QMessageBox(QMessageBox::Information, TITLE, "json server connected.").exec();
        } else {
            QMessageBox(QMessageBox::Information, TITLE, "conn fail.").exec();
        }
    }
}

void MainWindow::on_chatWindow_returnPressed()
{
    QString chatMsg = ui->chatWindow->text();
    // protocol format
    QString msg = "#json begin {";
    msg += "'type': 'chat',";
    msg += "'room': '#1',";
    msg += "'text': '" + chatMsg + "'";
    msg += "} #json end";
    jsonServer.write(msg.toUtf8().data()); // 가이드> toUtf8().data()는 toUtf8()호출시
    // 인스턴스가 즉시 만들어지고 사라지므로 data()의 리턴값을 바로 전달해야 합니다.

    // clear UI
    ui->chatWindow->setText("");
}

void MainWindow::recvEchoMsg()
{
    qint64 packetSize = jsonServer.bytesAvailable();
    bool isExistPackets = 0 < packetSize;
    //
    if(isExistPackets) {
        char* buffer = new char[packetSize + 1];
        jsonServer.read(buffer, packetSize);
        buffer[packetSize] = '\0'; // 가이드> 문자열의 끝을 넣어줘야 합니다.
        QMessageBox(QMessageBox::Information, TITLE, buffer).exec();
    } else {
        QMessageBox(QMessageBox::Information, TITLE, "msg recv fail.").exec();
    }

}
