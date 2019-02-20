#ifndef TRAYMENU_H
#define TRAYMENU_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class TrayMenu;
}

class TrayMenu : public QDialog
{
    Q_OBJECT

public:
    explicit TrayMenu(QWidget *parent = 0);
    ~TrayMenu();

private:
//    void focusOutEvent(QFocusEvent* e) override; // 오버라이드 명시
    void focusOutEvent(QFocusEvent* e) Q_DECL_OVERRIDE; // 오버라이드 명시

private slots:
    void on_onoff_toggled(bool checked);

    void on_gui_clicked();

    void on_exit_clicked();

private:
    Ui::TrayMenu *ui;
    MainWindow mWindow;
};

#endif // TRAYMENU_H
