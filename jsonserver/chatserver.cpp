#include "chatserver.h"
#include <QJsonDocument>

#define KOREAN(STR) QString::fromWCharArray(L##STR)

ChatServer::ChatServer()
{
    mRefWidget = nullptr;
    connect(this, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

void ChatServer::acceptPeer()
{
    //    "#json begin {\"type\": \"chat\", \"room\":\"abc\", \"name\":\"server\", \"text\":\"hello!\"} #json end";
    QTcpSocket* NewPeer = nextPendingConnection();
    auto NewPeerData = new PeerData();
    NewPeer->setUserData(0, NewPeerData);

    // NOTE: mRefWidget은 잠깐 참고하니, 지우지 않아도 된다.
    mRefWidget->addItem("손님이 입장하였습니다 : ");

    connect(NewPeer, SIGNAL(readyRead()), this, SLOT(readyPeer()));
    connect(NewPeer, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(errorPeer(QAbstractSocket::SocketError)));
}

void ChatServer::errorPeer(QAbstractSocket::SocketError e)
{
    QTcpSocket* curPeer = (QTcpSocket*) sender();
    PeerData* curPeerData = (PeerData*) curPeer->userData(0);

    if(0 < curPeerData->mRoomName.length())
        ExitRoom(curPeerData->mRoomName, curPeerData->mID);

    AddLog("손님이 서버에서 퇴장하였습니다 : " + QString::number(curPeerData->mID));
}

void ChatServer::readyPeer()
{
    mRefWidget->addItem("DATA들어옴~");

    QTcpSocket* curPeer = (QTcpSocket*) sender();
    PeerData* curPeerData = (PeerData*) curPeer->userData(0);

    auto PacketSize = curPeer->bytesAvailable();

    if(0 < PacketSize)
    {
        QByteArray NewPacket = curPeer->read(PacketSize);
        QString OneString = NewPacket;

        int EndPos = OneString.indexOf("#json end");
        if(EndPos != -1) {
            int BeginPos = OneString.indexOf("#json begin");
            if(BeginPos != -1) {
                BeginPos += 11;
                // 룸이름 얻기
                auto NewJson = QJsonDocument::fromJson(OneString.mid(BeginPos, EndPos - BeginPos).toUtf8());
                auto recvedRoomName = NewJson["room"].toString("global");

                // 자신의 룸이름이 잘라졌을 경우:
                // 1. 클라이언트 A 처음 메시지 : [userData="" // jsonData="RoomName1"] --> true
                // 1. 클라이언트 B 처음 메시지 : [userData="" // jsonData="RoomName1"] --> true
                if(curPeerData->mRoomName != recvedRoomName)
                {
                    // 이게 문제야! 이전 룸 이름이 존재한다면 방을 바꾸는 것이다!
                    // 1. 클라이언트 A 처음 메시지 : [userData="" // jsonData="RoomName1"] --> false
                    // 1. 클라이언트 B 처음 메시지 : [userData="" // jsonData="RoomName1"] --> false
                    if(0 < curPeerData->mRoomName.length())
                    {
                        ExitRoom(curPeerData->mRoomName, curPeerData->mID);
                    }
                    // 1. 클라이언트 A 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1"]
                    // 1. 클라이언트 B 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1"]
                    curPeerData->mRoomName = recvedRoomName;
                }

                // 룸찾기, 없으면 처음으로 만든다:
                // 1.   클라이언트 A 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = ""] --> true
                // 1-a. 클라이언트 B 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = ""] --> true
                // 1-b. 클라이언트 B 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = "RoomName1"] --> false
                if(!mRoomPool.contains(recvedRoomName))
                {
                    mRoomPool.insert(recvedRoomName, new RoomData());
                    AddLog("룸이 생성되었습니다 : " + recvedRoomName);
                }

                RoomData* CurRoom = mRoomPool.value(recvedRoomName);

                // 내가 그 룸에 없으면, 추가함
                // 1.   클라이언트 A 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = "RoomName1" // mPeer = ""] --> true
                // 1-a. 클라이언트 B 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = "RoomName1" // mPeer = ""] --> true
                // 1-b. 클라이언트 B 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = "RoomName1" // mPeer = "A"] --> true
                if(!CurRoom->mPeers.contains(curPeerData->mID)){
                    CurRoom->mPeers.insert(curPeerData->mID, curPeer);
                    AddLog("룸에 회원이 입장하였습니다 : " + QString::number(curPeerData->mID) + " >> " + recvedRoomName);
                }

                // 해당 룸에 존재하는 모든 피어에게 회람
                // 1.   클라이언트 A 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = "RoomName1" // mPeers = "A"] --> 본인메시지 수신.
                // 1-a. 클라이언트 B 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = "RoomName1" // mPeers = "A, B"] --> A와 본인메시지 수신.
                // 1-b. 클라이언트 B 처음 메시지 : [userData="RoomName1" // jsonData="RoomName1" // mRoomPool = "RoomName1" // mPeers = "A, B"] --> A와 본인메시지 수신.
                for(auto CurRoomPeer : CurRoom->mPeers) {
                    CurRoomPeer->write(NewPacket);
                }
            }
        }
    }
}

void ChatServer::SetLogWidget(QListWidget* widget)
{
    mRefWidget = widget;
}

void ChatServer::AddLog(QString text)
{
    mRefWidget->addItem(text);
    mRefWidget->scrollToBottom();
}

void ChatServer::ExitRoom(QString roomName, int peerID)
{
    if(mRoomPool.contains(roomName)) {
        AddLog("룸에서 회원이 퇴장하였습니다 : " + QString::number(peerID) + " << " + roomName);

        auto CurRoom = mRoomPool.value(roomName);
        CurRoom->mPeers.remove(peerID);
        if(CurRoom->mPeers.count() == 0)
        {
            mRoomPool.remove(roomName);
            AddLog("룸이 파괴되었습니다 : " + roomName);
        }
    }
}
