#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#define TITLE "DEBUG"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
        QMessageBox(QMessageBox::Information, TITLE, host).exec();

    }
}
