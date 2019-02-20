#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include "traymenu.h"

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    TrayIcon();

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason r);

public:
    TrayMenu mMenu;
};

#endif // TRAYICON_H
