#include "trayicon.h"
#include <QCursor>
#include <QGuiApplication>
#include <QScreen>

TrayIcon::TrayIcon() : mMenu()
{
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void TrayIcon::iconActivated(QSystemTrayIcon::ActivationReason r)
{
    switch(r) {
        case QSystemTrayIcon::Context:
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::MiddleClick:
        case QSystemTrayIcon::DoubleClick:
            {
                QPoint ClickPos = QCursor::pos();
                QRect ScreenRect = QGuiApplication::screenAt(ClickPos)->geometry();
                QSize MenuSize = mMenu.size();
                //
                int PosX = (ScreenRect.right() < ClickPos.x() + MenuSize.width()) ?
                            ClickPos.x() - MenuSize.width() : ClickPos.x();
                int PosY = (ScreenRect.bottom() < ClickPos.y() + MenuSize.height()) ?
                            ClickPos.y() - MenuSize.height() : ClickPos.y();
                //
                mMenu.move(PosX, PosY);
                mMenu.show();
                mMenu.activateWindow();
                mMenu.setFocus();
            }
            break;
        default:
            break;
    }
}
