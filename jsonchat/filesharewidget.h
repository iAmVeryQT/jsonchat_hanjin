#ifndef FILESHAREWIDGET_H
#define FILESHAREWIDGET_H

#include "mainwindow.h"

#include <QListWidgetItem>

class PropData: public QObjectUserData {
public:
    QString mSender;
    QString filePath;
};

class FileShareWidget : public QListWidgetItem, public QObject
{
public:
    FileShareWidget();
    FileShareWidget(const QString filePath, const QString name, const QString chat, const MainWindow* mainWindow);
    ~FileShareWidget();

    QWidget* customWidget;
    PropData* propData;
};

#endif // FILESHAREWIDGET_H
