#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>

class ChatServer : public QTcpServer
{
    Q_OBJECT

private slots:
    void acceptPeer();
    void readyPeer();
    void errorPeer(QAbstractSocket::SocketError e);

public:
    ChatServer();
    QMap<int, QTcpSocket*> mPeerMap; // 앞에는 키, 뒤에는 벨류
    // QMap의 내부 구현체는 RED-BLACK TREE
};

#endif // CHATSERVER_H

