#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include<QString>
#include <QObject>
#include<QUdpSocket>
#include<QFile>
#include<QThread>

class UdpReceiver : public QObject
{
    Q_OBJECT
public:
    UdpReceiver();
    ~UdpReceiver();
    void initSocket();
    void initFile(QString fileName);
    void setSaveFilePath(QString path);
    void setChatPort(int port);

public slots:
    void readPendingDatagrams();

signals:
    void resultReady(const QString &result);

private:
    QUdpSocket udpSocket;
    QFile file; //to store the received file
    int chatPort;
    QString saveFilePath;
};

#endif // UDPRECEIVER_H
