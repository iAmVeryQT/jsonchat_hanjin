#include "filesharewidget.h"

#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

#define KOREAN(STR) QString::fromWCharArray(L##STR)
#define DEBUG(STR) QMessageBox(QMessageBox::Information, "Debug", STR).exec();

FileShareWidget::FileShareWidget() {}
FileShareWidget::FileShareWidget(const QString filePath, const QString name, const QString chat, const MainWindow* mainWindow)
{
    this->setBackgroundColor(QColor(255, 0, 0));
    this->setSizeHint(QSize(0, 40));

    QPushButton* pButton = new QPushButton("파일받기");
    auto newFilePath = filePath;

    auto btnProp = new PropData();
    btnProp->mSender = name;
    btnProp->filePath = newFilePath;
    pButton->setUserData(0, btnProp);

    connect(pButton, SIGNAL( pressed() ), mainWindow, SLOT( onClickDNBtn() ));

    this->customWidget = new QWidget();
    QHBoxLayout* newLayout = new QHBoxLayout();
    newLayout->addWidget(new QLabel(chat));
    newLayout->addWidget(pButton);
    customWidget->setLayout(newLayout);
}

FileShareWidget::~FileShareWidget() {

}
