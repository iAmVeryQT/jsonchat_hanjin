#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QJsonDocument>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#define TITLE "DEBUG"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // enroll signal to slot
    connect(&jsonServer, SIGNAL(readyRead()), this, SLOT(recvEchoMsg()));
    // Alias
    MSG_TYPE["CHAT"] = "chat";
    MSG_SUB_TYPE["FILE_SHARE"] = "fileshare";
    MSG_SUB_TYPE["GET_FILE"] = "getFile";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connBtn_clicked()
{
    QString host = ui->hosturl->text();

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

void MainWindow::on_chatText_returnPressed()
{
    QString chatMsg = ui->chatText->text();
    QString roomName = ui->roomName->text();
    QString nickName = ui->nickName->toPlainText();

    // protocol format
    QString msg = "#json begin {";
    msg += "'type': " + MSG_TYPE["CHAT"] + ",";
    msg += "'room': '" + roomName + "',";
    msg += "'name': '" + nickName + "',";
    msg += "'text': '" + chatMsg + "'";
    msg += "} #json end";
    jsonServer.write(msg.toUtf8().data()); // 가이드> toUtf8().data()는 toUtf8()호출시
    // 인스턴스가 즉시 만들어지고 사라지므로 data()의 리턴값을 바로 전달해야 합니다.

    // clear UI
    ui->chatText->setText("");
}


class MyData: public QObjectUserData {
public:
    QString mSender;
    QString filePath;
};

void MainWindow::onDownloadBtn() {
    QPushButton* btn = (QPushButton*) sender();
    MyData* oneData = (MyData*) btn->userData(0);
    QMessageBox(QMessageBox::Information, TITLE, oneData->filePath).exec();
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
        //QMessageBox(QMessageBox::Information, TITLE, buffer).exec();

        QString jsonBuffer = buffer;
        delete[] buffer;
        jsonBuffer.remove("#json begin");
        jsonBuffer.remove("#json end");

        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonBuffer.toUtf8());
        QString subType = jsonDoc["subtype"].toString("");
        QString name = jsonDoc["name"].toString("아무개");
        QString chat = jsonDoc["text"].toString("");

        //////////////////
        if(subType == MSG_SUB_TYPE["FILE_SHARE"]) {
            ui->talkList->addItem("---");
            QListWidgetItem* customItem = new QListWidgetItem();
            customItem->setBackgroundColor(QColor(255, 0, 0));
            customItem->setSizeHint(QSize(0, 40));
            ui->talkList->addItem(customItem);
            //
            QPushButton* pButton = new QPushButton("파일받기");
            auto newFilePath = jsonDoc["filepath"].toString();
            auto newData = new MyData();
            newData->mSender = name;
            newData->filePath = newFilePath;
            pButton->setUserData(0, newData);
            // wiring
            connect(pButton, SIGNAL(pressed()), this, SLOT(onDownloadBtn()));
            //
            QWidget* customWidget = new QWidget();
            QHBoxLayout* newLayout = new QHBoxLayout();
            newLayout->addWidget(new QLabel("adsf"));
            newLayout->addWidget(pButton);
            //
            customWidget->setLayout(newLayout);
            //
            ui->talkList->setItemWidget(customItem, customWidget);
        } else if(subType == MSG_SUB_TYPE["GET_FILE"]) {
            // TODO: 파일 받는 코드
        } else {
            QString basePath = QCoreApplication::applicationDirPath();
            QIcon img(basePath + "../../../../../image/in-love.png");
            QListWidgetItem* imoji = new QListWidgetItem(img, "[" + name + "] " + chat);
            // QListWidgetItem 소멸은 자동으로 해줌
            ui->talkList->addItem(imoji);
        }
        //////////////////
        // 스크롤 아래로
        ui->talkList->scrollToBottom();
    } else {
        QMessageBox(QMessageBox::Information, TITLE, "msg recv fail.").exec();
    }
}

void MainWindow::on_sendFile_clicked()
{
    QString roomName = ui->roomName->text();
    QString nickName = ui->nickName->toPlainText();
    //
    QFileDialog Dialog(nullptr, "전송할 파일을 선택하세요.");
    //
    if(Dialog.exec()) {
        const QString filePath = Dialog.selectedFiles()[0];
        const int64_t fileSize = QFileInfo(filePath).size();
        const int64_t lIdx = filePath.lastIndexOf("/");
        const QString shortPath = filePath.right(filePath.length() - lIdx + 1);
        //
        QMessageBox(QMessageBox::Information, TITLE, filePath).exec();
        //
        QString msg = "#json begin {";
        msg += "'type': " + MSG_TYPE["CHAT"] + ",";
        msg += "'room': '" + roomName + "',";
        msg += "'name': '" + nickName + "',";
        msg += "'text': '" + nickName + "님이";
        msg += "(" + shortPath + "," + QString::number(fileSize) + "byte) 파일을 공유합니다." + "',";
        msg += "'subtype': " + MSG_SUB_TYPE["FILE_SHARE"] + ",";
        msg += "'filepath': '" + filePath + "',";
        msg += "'filesize': '" + QString::number(fileSize) + "',";
        msg += "} #json end";
        //
        jsonServer.write(msg.toUtf8().data());
        //
    } else {
        QMessageBox(QMessageBox::Information, TITLE, "파일 전송이 취소 되었습니다.").exec();
    }
}

void MainWindow::on_getItem_released()
{
    auto list = ui->talkList->selectedItems();
    if(0 < list.length()){
        const QString item = list[0]->text();
        QMessageBox(QMessageBox::Information, TITLE, item).exec();
        ui->chatText->setText(item);
    }
}
