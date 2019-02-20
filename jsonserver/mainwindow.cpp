#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <QCloseEvent>

struct DD{
    char a;
    int b;
    char c;
    char d;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //
    #ifdef Q_OS_WIN
        setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    #else
        setFixedSize(size());
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    #endif
    //
    DD dd;
    qDebug() << sizeof(dd);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetServerOn() {
    qDebug() << "in SetServerON";
//    QString qHostAddr = "192.168.100.1";
    if(!mServer.isListening()) {
        mServer.listen(QHostAddress::Any, 10125);
        qDebug() << "ServerOn!!!";
    }
}

void MainWindow::SetServerOff() {
    qDebug() << "in SetServerOFF";
    if(mServer.isListening()) {
        mServer.close();
        qDebug() << "ServerOff!!!";
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    hide();
    e->ignore();
}
