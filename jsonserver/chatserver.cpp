#include "chatserver.h"
#include <QTcpSocket>

ChatServer::ChatServer()
{
    connect(this, SIGNAL(newConnection()) ,SLOT(acceptPeer()));
}

class PeerData: public QObjectUserData {
    public:
        PeerData() : mID(MakeID())
        {       }
        ~PeerData()
        {       }
    public:
        const int mID;
        int MakeID()
        {
            static int lastID = -1; // 내부적으로 모든 클래스의 ID의 관리가 가능하다.
            return ++lastID;
        }
};

void ChatServer::acceptPeer()
{
    QString msg = "#json begin {\"type\": \"chat\", \"room\":\"abc\", \"name\":\"server\", \"text\":\"hello!\"} #json end";
    //
    QTcpSocket* A = nextPendingConnection(); // 만든 놈이 삭제 처리함.
    auto B = new PeerData();
    A->setUserData(0, B); // A는 항상 B의 데이터를 갖고 다닌다.
    mPeerMap.insert(B->mID, A);
    //
    A->write(msg.toUtf8());
    //
    connect(A, SIGNAL(readyRead()), this, SLOT(readyPeer()));
    // 에러 처리
    connect(A, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorPeer));
}

void ChatServer::errorPeer(QAbstractSocket::SocketError e)
{
    // 에러처리를 readyPeer보다 먼저 코딩하는 습관 중요.
    auto CurPeer = (QTcpSocket*) sender(); // 슬롯을 받은 사람을 알 수 있다.
    auto CurData = (PeerData*)CurPeer->userData(0);
    // CurData는 부모의 virtual table pointer에 의해서 삭제됨.
    mPeerMap.remove(CurData->mID); // 피어제거.
}

void ChatServer::readyPeer()
{
    // TODO: 세부적인 구현.
    QTcpSocket* A = (QTcpSocket*) sender();
    auto curData = A->userData(0);
    qint64 len = A->bytesAvailable();
    if(len > 0)
    {
        QByteArray c = A->read(len);
        //
        A->write(c); // 한명한테만 반사.
        // 브로드 캐스트.
        for(auto& iPeer: mPeerMap) { // auto를 쓸때는 &를 적어주는 습관.
           iPeer->write(0);
        }
    }
}
