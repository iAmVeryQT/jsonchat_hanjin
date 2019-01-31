#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QJsonDocument>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#define TITLE "DEBUG"
// ##은 파라미터 식별자와 문자열을 합치기 위한 키워드.
#define KOREAN(STR) QString::fromWCharArray(L##STR)
#define DEBUG(STR) QMessageBox(QMessageBox::Information, "Debug", STR).exec();

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
    MSG_SUB_TYPE["GET_FILE"] = "getfile";
    MSG_SUB_TYPE["SET_FILE"] = "setfile";
}

MainWindow::~MainWindow()
{
    delete ui;
    // 링크드리스트의 경우에도, i를 게속 접근하거나 할당을 매번하지 않고 루핑.
    foreach (auto& iFile, mFileList) {
        if(iFile != nullptr)
        {
            iFile->remove();
            delete iFile;
        }
    }
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
    // SLOT 메시지는 sender를 통해서 출처를 알 수 있다.
    // QPushButton* btn = (QPushButton*) sender();
    if(auto btn = qobject_cast<QPushButton*>(sender())) {
        QFileDialog Dialog(nullptr, KOREAN("전송받을 폴더 알려주세요"));
        Dialog.setFileMode(QFileDialog::Directory);
        // 실행 정지
        if(Dialog.exec()) {
            // 선택된 폴더
            const QString dirPath = Dialog.selectedFiles()[0];
            // getFile
            QString roomName = ui->roomName->text();
            QString nickName = ui->nickName->toPlainText();
            //
            MyData* propData = (MyData*) btn->userData(0);
            QString fileSender = propData->mSender;
            QString filePath = propData->filePath;
            //QString fileID   = "0";
            //
            filePath = filePath.replace("\\", "/");
            int slashPos = filePath.lastIndexOf("/");
            //
            QString shortName;
            //
            if(slashPos == -1) {
                shortName = filePath;
            } else {
                // [a] [/] [b]
                shortName = filePath.right(filePath.length() - slashPos - 1);
            }
            //
            QFile* writeFile = new QFile(dirPath + "/" + shortName + ".download");
            //
            if(writeFile->open(QFileDevice::WriteOnly)) {
                int fileID = mFileList.length();
                mFileList.append(writeFile);
                //
                QString msg = "#json begin {";
                msg += "'type': '" + MSG_TYPE["CHAT"] + "',";
                msg += "'room': '" + roomName + "',";
                msg += "'name': '" + nickName + "',";
                msg += "'text': '" + nickName + "님이 파일받기를 시작합니다." + "',";
                msg += "'subtype' : '" + MSG_SUB_TYPE["GET_FILE"] + "',";
                msg += "'sender'  : '" + fileSender + "',";
                msg += "'filepath': '" + filePath + "',";
                msg += "'fileid'  : '" + QString::number(fileID) + "'";
                msg += "} #json end";
                jsonServer.write(msg.toUtf8().constData());
            }
            //
//            QString msg = "#json begin {";
//            msg += "'type': '" + MSG_TYPE["CHAT"] + "',";
//            msg += "'room': '" + roomName + "',";
//            msg += "'name': '" + nickName + "',";
//            msg += "'text': '" + nickName + "님이 파일받기를 시작합니다." + "',";
//            msg += "'subtype' : '" + MSG_SUB_TYPE["GET_FILE"] + "',";
//            msg += "'sender'  : '" + fileSender + "',";
//            msg += "'filepath': '" + filePath + "',";
//            msg += "'fileid'  : '" + QString::number(fileID) + "'";
//            msg += "} #json end";
//            jsonServer.write(msg.toUtf8().constData());
            //
            //
//            QMessageBox(QMessageBox::Information, TITLE, dirPath).exec();
        }
    }
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
        if(subType == MSG_SUB_TYPE["FILE_SHARE"]) { // 파일 공유 공지 패킷
            // >>>TODO: 파일 공유 위젯 리팩토링
            // 파일 공유를 위한 위젯 생성
            QListWidgetItem* customItem = new QListWidgetItem();
            customItem->setBackgroundColor(QColor(255, 0, 0));
            customItem->setSizeHint(QSize(0, 40));
            ui->talkList->addItem(customItem);
            // 버튼 UI 생성
            QPushButton* pButton = new QPushButton("파일받기");
            auto newFilePath = jsonDoc["filepath"].toString();
            // 버튼의 프로퍼티 생성
            auto btnProp = new MyData();
            btnProp->mSender = name;
            btnProp->filePath = newFilePath;
            pButton->setUserData(0, btnProp);
            // SIGNAL -> SLOT wiring
            connect(pButton, SIGNAL(pressed()), this, SLOT(onDownloadBtn()));
            //
            QWidget* customWidget = new QWidget();
            QHBoxLayout* newLayout = new QHBoxLayout();
            newLayout->addWidget(new QLabel(chat));
            newLayout->addWidget(pButton);
            // <<<파일 공유 위젯 리팩토링
            customWidget->setLayout(newLayout);
            //
            ui->talkList->setItemWidget(customItem, customWidget);
            //
        } else if(subType == MSG_SUB_TYPE["SET_FILE"]) { // 파일 전송 패킷
            auto newItem = new QListWidgetItem("<" + name + ">" + chat);
            newItem->setBackgroundColor(QColor(224, 255, 192));
            ui->talkList->addItem(newItem);
            //
            auto fileID = jsonDoc["fileid"].toString().toInt();
            auto done = jsonDoc["done"].toString().toInt();
            // TODO: 파일 받는 코드
            auto Base64 = jsonDoc["base64"].toString();
            //
            if(auto curFile = mFileList.at(fileID)) {
                QByteArray aa(Base64.toUtf8().constData());
                // BASE64를 디코딩하는 부분
                QByteArray buff = QByteArray::fromBase64(aa);
                curFile->write(buff);
                //
                if(done == 1) {
                    curFile->close();
                    auto oldName = curFile->fileName();
                    curFile->rename(oldName.left(oldName.length() - 9));
                    delete curFile;
                    mFileList.replace(fileID, nullptr);
                } else {

                }
            }
        } else { // 이모지 채팅 패킷
            // TODO: 서브 타입이 없는 패킷
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
        msg += "'type': '" + MSG_TYPE["CHAT"] + "',";
        msg += "'room': '" + roomName + "',";
        msg += "'name': '" + nickName + "',";
        msg += "'text': '" + nickName + "님이";
        msg += "(" + shortPath + "," + QString::number(fileSize) + "byte) 파일을 공유합니다." + "',";
        msg += "'subtype' : '" + MSG_SUB_TYPE["FILE_SHARE"] + "',";
        msg += "'filepath': '" + filePath + "',";
        msg += "'filesize': '" + QString::number(fileSize) + "',";
        msg += "} #json end";
        //
        DEBUG(msg);
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
