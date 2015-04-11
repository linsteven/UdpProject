#ifndef UDPSENDER_H
#define UDPSENDER_H
#include<QString>
#include <QObject>
#include<QUdpSocket>
#include<QFile>
#include<QThread>

class UdpSender : public QObject
{
    Q_OBJECT
public:
    UdpSender();
    ~UdpSender();
    int sendFile(QString filePath, QString fileName, QString userName, QString ip, QString myIp, QString broadcast);
    void sendFileRequest(QString filePath,QString fileName,QString userName, QString ip,QString myIp);
    void sendMesg(QString strMesg, QString userName,QString ip);
    void sendFileResponse(bool isAccepted, QString userName,QString ip);
    void setChatPort(int port);
    void sendOnline(QString userName, QString myIp,QString broadcast);

signals:
    void sendFileProgress(const QString &);
    void signalSendOnline();

private:
    QUdpSocket *udpSocket;
    QFile *file;
    int sendBlockSize;
    int chatPort;
};

#endif // UDPSENDER_H
