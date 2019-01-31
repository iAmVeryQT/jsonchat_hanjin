#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filesharewidget.h"
//
#include <QMessageBox>
#include <QJsonDocument>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLabel>
#include <QThread>
//
#define TITLE "DEBUG"
// ##은 파라미터 식별자와 문자열을 합치기 위한 키워드.
#define KOREAN(STR) QString::fromWCharArray(L##STR)
#define DEBUG(STR) QMessageBox(QMessageBox::Information, "Debug", STR).exec();
#define DEBUG_UI(STR) ui->talkList->addItem(STR);
// 호스트 포트
#define HOST_PORT 10125
// 연결 대기 시간
#define WAIT_TIME 5000
// 채팅 메시지
#define CONNECTED_MSG "서버에 접속 되었습니다."
#define CONNECT_FAIL_MSG "서버에 실패하였습니다. 다시 시도해 주세요."
#define DN_MSG(NICK_NAME) NICK_NAME + "님이 파일받기를 시작합니다."
// 프로토콜 관련
#define P_START_FLAG "#json begin"
#define P_START_FLAG_SIZE 11
#define P_END_FLAG "#json end"
#define P_END_FLAG_SIZE 9
// 파일명 관련
#define DN_FILE_POSTFIX ".download"
//
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 시그널->슬롯 등록
    QObject::connect(&jsonServer, SIGNAL(readyRead()), this, SLOT(receiveMsgFromServer()));
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
    foreach (auto& iterFile, dnIngFileList) {
        if(iterFile != nullptr)
        {
            iterFile->remove();
            delete iterFile;
        }
    }
}
// 연결 버튼 클릭 핸들러
void MainWindow::on_connBtn_clicked()
{
    // 호스트명 저장/UI에 반영
    this->host = ui->hosturl->text();
    ui->hosturl->setText(this->host);
    // 호스트에 접속
    jsonServer.connectToHost(host, HOST_PORT);
    // waiting conn for 5 sec
    if(jsonServer.waitForConnected(WAIT_TIME)) {
        ui->talkList->addItem(CONNECTED_MSG);
    } else {
        ui->talkList->addItem(CONNECT_FAIL_MSG);
    }
}
// 채팅 문자열 전송 핸들러
void MainWindow::on_chatText_returnPressed()
{
    QString chatMsg = ui->chatText->text();
    QString roomName = ui->roomName->text();
    QString nickName = ui->nickName->toPlainText();
    // 채팅 메시지 전송 문자열
    QString msg = P_START_FLAG;
    msg += "{";
    msg += "'type': " + MSG_TYPE["CHAT"] + ",";
    msg += "'room': '" + roomName + "',";
    msg += "'name': '" + nickName + "',";
    msg += "'text': '" + chatMsg + "'";
    msg += "}";
    msg += P_END_FLAG;
    jsonServer.write(msg.toUtf8().data()); // 가이드> toUtf8().data()는 toUtf8()호출시
    // 인스턴스가 즉시 만들어지고 사라지므로 data()의 리턴값을 바로 전달해야 합니다.
    //
    // 입력창 청소
    ui->chatText->setText("");
}
// 다운로드 버튼 클릭 콜백
void MainWindow::onClickDNBtn() {
    QString roomName = ui->roomName->text();
    QString nickName = ui->nickName->toPlainText();
    int fileID = dnIngFileList.length();
    //
    // NOTE: SLOT 메시지는 sender를 통해서 출처를 알 수 있다.
    if(auto btn = qobject_cast<QPushButton*>(sender())) {
//    if(clickedBtn) {
        QFileDialog dnPathDialog(nullptr, KOREAN("전송받을 폴더 알려주세요"));
        dnPathDialog.setFileMode(QFileDialog::Directory);
        //
        if(dnPathDialog.exec()) { // NOTE: 다이얼로그 블로킹 모드
            const QString dnPath = dnPathDialog.selectedFiles()[0]; // 다운로드 할 폴더 경로
            DEBUG_UI(dnPath + " 폴더로 다운로드 받겠습니다.");
            // 빈 파일 공유 위젯 생성
            FileShareWidget *fileShareWidget = new FileShareWidget();
            fileShareWidget->propData = (PropData*) btn->userData(0);
//            fileShareWidget->propData = (PropData*) clickedBtn->userData(0);
            QString propSender = fileShareWidget->propData->mSender;
            QString propFilePath = fileShareWidget->propData->filePath;
            //QString fileID   = "0";
            //
            propFilePath = propFilePath.replace("\\", "/");
            int slashPos = propFilePath.lastIndexOf("/");
            //
            QString shortName;
            //
            if(slashPos == -1) {
                shortName = propFilePath;
            } else {
                // exam: c:/windows/fonts /dotum.ttf
                //                       ^
                //                       |
                //                      pos
                shortName = propFilePath.right(propFilePath.length() - slashPos - 1);
            }
            DEBUG_UI("다운로드 받을 파일은 " + dnPath + "/" + shortName + DN_FILE_POSTFIX + "입니다.")
            QFile* dnCloneFile = new QFile(dnPath + "/" + shortName + DN_FILE_POSTFIX);
            //
            if(dnCloneFile->open(QFileDevice::WriteOnly)) {
                dnIngFileList.append(dnCloneFile);
                //
                QString msg = P_START_FLAG;
                msg += "{";
                msg += "'type': '" + MSG_TYPE["CHAT"] + "',";
                msg += "'room': '" + roomName + "',";
                msg += "'name': '" + nickName + "',";
                msg += "'text': '" + DN_MSG(nickName) + " ',";
                msg += "'subtype' : '" + MSG_SUB_TYPE["GET_FILE"] + "',";
                msg += "'sender'  : '" + propSender + "',";
                msg += "'filepath': '" + propFilePath + "',";
                msg += "'fileid'  : '" + QString::number(fileID) + "'";
                msg += "}";
                msg += P_END_FLAG;
                //
                DEBUG_UI(msg)
                jsonServer.write(msg.toUtf8().constData());
            }
        }
    } else {
        DEBUG_UI("버튼이 없습니다.")
    }
}
// JSON 캡쳐 받기
void MainWindow::captureJSON(const QJsonDocument jsonDoc) {
    DEBUG_UI(jsonDoc["text"].toString("captureJSON"))
}
// 스트리밍 메시지 처리
void MainWindow::processStreamMsg(const char* data)
{
    receivedData += data;
    int beginPos = 0, endPos = 0;
    while((endPos = receivedData.indexOf("#json end")) != -1)
    {
        beginPos = receivedData.indexOf("#json begin");
        if(beginPos != -1) {
            int startPos = beginPos + P_START_FLAG_SIZE;
            QString json = receivedData.mid(startPos,
                                            endPos - startPos);
            captureJSON(QJsonDocument::fromJson(json.toUtf8()));
            receivedData.remove(0, endPos + P_START_FLAG_SIZE);
        } else {
            DEBUG_UI("프로토콜의 시작 지점이 유실되었습니다.")
        }
    }
}
// 서버로부터 메시지를 받는 부분
void MainWindow::receiveMsgFromServer()
{
    DEBUG_UI("서버로부터 메시지를 받고 있습니다.")
    qint64 packetSize = jsonServer.bytesAvailable();
    bool isExistPackets = 0 < packetSize;
    if(isExistPackets) {
        char* buffer = new char[packetSize + 1];
        jsonServer.read(buffer, packetSize);
        buffer[packetSize] = '\0'; // 가이드> 문자열의 끝을 넣어줘야 합니다.
        //
        processStreamMsg(buffer);
        //
        QString jsonBuffer = buffer;
        delete[] buffer;
        jsonBuffer.remove("#json begin");
        jsonBuffer.remove("#json end");
        //
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonBuffer.toUtf8());
        QString type     = jsonDoc["type"].toString("default type");
        QString room     = jsonDoc["room"].toString("123");
        QString name     = jsonDoc["name"].toString("아무개");
        QString text     = jsonDoc["text"].toString("default text");
        QString subType  = jsonDoc["subtype"].toString("default subtype");
        QString sender   = jsonDoc["sender"].toString("default sender");
        QString filePath = jsonDoc["filepath"].toString("default file path");
        QString fileID   = jsonDoc["fileID"].toString("default ID");
        QString fileSize = jsonDoc["filesize"].toString("default file size");
        //
        if(subType == MSG_SUB_TYPE["FILE_SHARE"]) { // FILE_SHARE 패킷이라면,
            // 파일 공유 공지
            FileShareWidget* fileShareWidget = new FileShareWidget(filePath, name, text, this);
            ui->talkList->addItem(fileShareWidget);
            ui->talkList->setItemWidget(fileShareWidget, fileShareWidget->customWidget);
        } else if(subType == MSG_SUB_TYPE["GET_FILE"]) { // GET_FILE 패킷이라면,
            // 파일 받기
            auto dnFileWidget = new QListWidgetItem("<" + name + ">" + text);
            dnFileWidget->setBackgroundColor(QColor(224, 255, 192));
            ui->talkList->addItem(dnFileWidget);
            //
            if(sender != "default sender" && sender == name) { // 보내는 사람이 본인일 경우,
                // 파일 전송하기
                QString fullPath = QStandardPaths::standardLocations(
                            QStandardPaths::DownloadLocation
                            ).value(0) + "/" + filePath;
                DEBUG_UI("파일 전송하기:" + fullPath);
                QFile* MyFile = new QFile(fullPath);
                if(MyFile->open(QFileDevice::ReadOnly)) {
                    auto fileSize = MyFile->size();
                    if(fileSize < 1024 * 100 ) { // 100K 이하일 경우엔, 한방에 보냄.
                        QByteArray NewBuffer = MyFile->read(fileSize);
                        QByteArray NewBase64 = NewBuffer.toBase64();
                        MyFile->close();

                        QString msg = P_START_FLAG;
                        msg += "{";
                        msg += "'type':'chat',";
                        msg += "'room':'" + room + "',";
                        msg += "'name':'" + name + "',";
                        msg += "'text':'송신완료',";
                        msg += "'subtype':'setfile',";
                        msg += "'fileid':'" + fileID + "',";
                        msg += QString("'base64':'") + NewBase64.constData() + "',";
                        msg += "'done':'1'";
                        msg += "}";
                        msg += P_END_FLAG;
                        //
                        jsonServer.write(msg.toUtf8().constData());
                    } else {
                        // 쓰레드 = 함수를 호출 하는데 비동기로 호출 using 람다
                        QThread* NewThread = QThread::create([this, MyFile, fileSize, fileID, room, name]
                        {
                            auto SendSize = fileSize;
                            while(0 < SendSize)
                            {
                                auto BlockSize = 1024 * 10;
                                if(SendSize < BlockSize) {
                                    BlockSize = SendSize;
                                }
                                SendSize -= BlockSize;
                                //
                                QByteArray NewBuffer = MyFile->read(BlockSize);
                                QByteArray NewBase64 = NewBuffer.toBase64();
                                //
                                QString json = "#json begin {";
                                json += "'type':'chat',";
                                json += "'room':'" + room + "',";
                                json += "'name':'" + name + "',";
                                if(SendSize == 0) {
                                    json += KOREAN("'text':'송신완료!!!',");
                                } else {
                                    QString Info;
                                    Info.sprintf("[%d/%d]", fileSize - SendSize);
                                    json += KOREAN("'text':'송신중") + Info + "',";
                                }
                                json += "'text':'송신중...!!!',";
                                json += "'subtype':'setfile',";
                                json += "'fileid':'" + fileID + "',";
                                json += QString("'base64':'") + NewBase64.constData() + "',";
                                if(SendSize == 0) {
                                    json += "'done':'1'";
                                } else {
                                    json += "'done':'0'";
                                }
                                json += "} #json end";
                                //
                                jsonServer.write(json.toUtf8().constData());
                                jsonServer.waitForBytesWritten(5000);
                            }
                            MyFile->close();
                            delete MyFile;
                        });
                        NewThread->start();
                    }
                }
                ui->talkList->addItem("this is me!!!");
            }
        } else if(subType == MSG_SUB_TYPE["SET_FILE"]) { // 파일 수신 패킷
            /// >>> 여기부터 다시 보기.
            auto newItem = new QListWidgetItem("<" + name + ">" + text);
            newItem->setBackgroundColor(QColor(224, 255, 192));
            ui->talkList->addItem(newItem);
            //
            auto fileID = jsonDoc["fileid"].toString().toInt();
            auto done = jsonDoc["done"].toString().toInt();
            // TODO: 파일 받는 코드
            auto Base64 = jsonDoc["base64"].toString();
            //
            if(auto curFile = dnIngFileList.at(fileID)) {
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
                    dnIngFileList.replace(fileID, nullptr);
                } else {

                }
            }
        } else { // 이모지 채팅 패킷
            // TODO: 서브 타입이 없는 패킷
            QString basePath = QCoreApplication::applicationDirPath();
            QIcon img(basePath + "../../../../../image/in-love.png");
            QListWidgetItem* imoji = new QListWidgetItem(img, "[" + name + "] " + text);
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
        DEBUG_UI(filePath)
        const int64_t fileSize = QFileInfo(filePath).size();
        const int64_t lIdx = filePath.lastIndexOf("/");
        const QString shortPath = filePath.right(filePath.length() - lIdx - 1);
        DEBUG_UI("전송할 파일의 shortPath:" + shortPath);
        //
        QString msg = P_START_FLAG;
        msg += "{";
        msg += "'type': '" + MSG_TYPE["CHAT"] + "',";
        msg += "'room': '" + roomName + "',";
        msg += "'name': '" + nickName + "',";
        msg += "'text': '" + nickName + "님이";
        msg += "(" + shortPath + "," + QString::number(fileSize) + "byte) 파일을 공유합니다." + "',";
        msg += "'subtype' : '" + MSG_SUB_TYPE["FILE_SHARE"] + "',";
        msg += "'filepath': '" + filePath + "',";
        msg += "'filesize': '" + QString::number(fileSize) + "',";
        msg += "}";
        msg += P_END_FLAG;
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
