#include "udpsender.h"
#include <QDebug>
#include<QtGlobal>
udpSender::udpSender()
{
    udpSocket = new QUdpSocket(this);
    file = new QFile("");
    sendBlockSize = 8500;
    chatPort = 7050;
}

udpSender::~udpSender()
{

}

int udpSender::sendFile(QString filePath, QString fileName, QString userName, QString ip,QString myIp,QString broadcast)
{
    //send template:
    //   "2#userName#fileName#blockId#percent#blockData"
    if(filePath.endsWith('/'))file->setFileName(filePath+fileName);
    else file->setFileName(filePath+'/'+fileName);
    int k=0;
    if (!file->open(QIODevice::ReadOnly))
        return -1;//failed
    QString strHead = "2#" + userName +"#"+ fileName +"#";
    int blockNum = file->size()/sendBlockSize +1;
    int percent;
    for(int i=0;i<file->size()/sendBlockSize;i++){
        QByteArray data = file->read(sendBlockSize);
        QByteArray sendByteArray;
        sendByteArray.append(strHead.toUtf8());
        percent = (i+1)*1.0/blockNum*100;
        QString strPer = QString::number(i+1,10)+"#"+QString::number(percent,10)+"#";
        sendByteArray.append(strPer.toUtf8());
        //文件数据部分不转码的话，由于接收时统一fromUtf8，会乱码
        QString strData = QString(data);
        sendByteArray.append(strData.toUtf8());
        udpSocket->writeDatagram(sendByteArray,QHostAddress(ip),7050);
        k++;//test
        //qDebug()<<"k="<<k;
        qDebug()<<"send strHead="<<strHead<<" strPer="<<strPer;
        if((i+1)%5== 120){
            sendOnline(userName,myIp,broadcast);//发文件时也要发在线报文，不然会下线
            //qDebug()<<"send online"<<endl;
    }
        emit sendFileProgress(fileName+" 正在发送..."+QString::number(percent,10)+"%");
        QThread::msleep(20);
    }
    if(file->size()%sendBlockSize != 0){
        QByteArray data = file->read(file->size()%sendBlockSize);//(file->size()-file->size()/sendBlockSize*sendBlockSize);
        QByteArray sendByteArray;
        sendByteArray.append(strHead.toUtf8());
        //percent = (i+1)*1.0/blockNum*100;
        QString strPer = QString::number(blockNum,10)+"#100#";
        sendByteArray.append(strPer.toUtf8());
        sendByteArray.append(data);

        udpSocket->writeDatagram(sendByteArray,QHostAddress(ip),7050);
        qDebug()<<"strHead="<<strHead<<" strPer="<<strPer;
        k++;//test
    }
    emit sendFileProgress(fileName+" 发送完成...100%");
    return 0;
    //qDebug()<<" send times: "<<k<<"\n send file ok!"<<"blockNum="<<blockNum;
}

void udpSender::sendFileRequest(QString filePath, QString fileName, QString userName, QString ip, QString myIp)
{
    //send request for sending file
    //example: "3#userName#fileName#strSize#myIp"
    if(filePath.endsWith('/'))file->setFileName(filePath+fileName);
    else file->setFileName(filePath+'/'+fileName);
    //file->setFileName(filePath + fileName);
    //qDebug()<<filePath+fileName;
    long long size = file->size();
    long long sizeKB = size/1024 > 0 ? size/1024 : 1;
    //if(sizeKB == 0)sizeKB = 1;
    QString strSize;
    double sizeMB;
    if(sizeKB>1023){
        sizeMB = sizeKB/1024.0;
        strSize = QString::number(sizeMB,'f',2) + "MB";
    }else{
        strSize  = QString::number(sizeKB) + "KB";
    }
    QString strSend = "3#" + userName + "#" + fileName + "#" + strSize + "#"+myIp;
    QByteArray sendByteArray;
    sendByteArray.append(strSend.toUtf8());
    udpSocket->writeDatagram(sendByteArray,QHostAddress(ip),chatPort);
    qDebug()<<"request string :"<<strSend;
}

void udpSender::sendMesg(QString strMesg, QString userName, QString ip)
{
    QString strSend = "1#" + userName + "#" + strMesg;
    QByteArray sendByteArray;
    sendByteArray.append(strSend.toUtf8());
    udpSocket->writeDatagram(sendByteArray,QHostAddress(ip),chatPort);
    qDebug()<<"send: "<<strSend;
}

void udpSender::sendFileResponse(bool isAccepted, QString userName, QString ip)
{
    /*  responce for sending file request
        example/template: "4#userName#0"
        userName can be used to ensure the responcer is right;
        the fileName,path  may be needed to polish the project in the furture
    */
    QString strSend = "4#" + userName + "#";
    if(isAccepted) strSend += "1";
    else strSend += "0";
    QByteArray sendByteArray;
    sendByteArray.append(strSend.toUtf8());
    udpSocket->writeDatagram(sendByteArray,QHostAddress(ip),chatPort);
    qDebug()<<"fileResponce: "<<strSend;
}

void udpSender::setChatPort(int port)
{
    chatPort = port;
}

void udpSender::sendOnline(QString userName, QString myIp, QString broadcast)
{
    //send broadcast message to inform others of my online status
    QString strSend = "5#"+userName+"#"+myIp;
    QByteArray sendByteArray;
    sendByteArray.append(strSend.toUtf8());
    udpSocket->writeDatagram(sendByteArray,QHostAddress(broadcast),chatPort);
}
