#include "traymenu.h"
#include "ui_traymenu.h"

#include <QDebug>

TrayMenu::TrayMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrayMenu)
{
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // WindowStaysOnTopHing : 최상위로 뜨도록 설정.
    ui->setupUi(this);
}

TrayMenu::~TrayMenu()
{
    delete ui;
}

void TrayMenu::on_onoff_toggled(bool checked)
{
    qDebug() << "onoff_toggled";

    if(checked) {
        qDebug() << "setServerOn";
        mWindow.SetServerOn();
    } else {
        qDebug() << "setServerOff";
        mWindow.SetServerOff();
    }
}

void TrayMenu::on_gui_clicked()
{
    mWindow.show();
}

void TrayMenu::on_exit_clicked()
{
    QApplication::exit(0);
}

void TrayMenu::focusOutEvent(QFocusEvent*) {
    hide();
//    qDebug() << "AAAAAA";
}
