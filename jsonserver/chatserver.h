#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QListWidget>

class PeerData : public QObjectUserData
{
public:
    PeerData() : mID(MakeID())
    {
    }
private:
    static int MakeID() // 싱글톤 방식
    {
        static int lastID = -1;
        return ++lastID;
    }

public:
    const int mID;
    QString mRoomName;
};

class RoomData
{
public:
    // QTcpSocket* = 4바이트
    QMap<int, QTcpSocket*> mPeers;
};

class ChatServer : public QTcpServer
{
    Q_OBJECT

public:
    ChatServer();
private slots:
    void acceptPeer();
    void readyPeer();
    void errorPeer(QAbstractSocket::SocketError e);

public:
    void SetLogWidget(QListWidget* widget);
    void AddLog(QString text);
    void ExitRoom(QString roomName, int peerID);
private:
    QListWidget* mRefWidget;
    QMap<QString, RoomData*> mRoomPool; // 앞에는 키, 뒤에는 벨류
    // QMap의 내부 구현체는 RED-BLACK TREE
};

#endif // CHATSERVER_H

