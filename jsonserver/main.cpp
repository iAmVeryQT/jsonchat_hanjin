#include "trayicon.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //
    QString ResPath = QCoreApplication::applicationDirPath() + "/../../assets/";
    QIcon MyIconPng(ResPath + "jsonserver_icon.png");
    //
    TrayIcon MyIcon;
    MyIcon.setIcon(MyIconPng);
    MyIcon.show();
    //
    return a.exec();
}
