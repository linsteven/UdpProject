#include "udpreceiver.h"
#include<QMetaObject>
#include<QDebug>
UdpReceiver::UdpReceiver()
{
    chatPort = 7050;
    initSocket();
    saveFilePath = "/";
}

UdpReceiver::~UdpReceiver()
{

}

void UdpReceiver::initSocket()
{
    udpSocket.bind(QHostAddress::Any, chatPort);
    connect(&udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void UdpReceiver::initFile(QString fileName)
{
    //file.setFileName(saveFilePath+"/"+fileName);
    if(saveFilePath.endsWith('/'))file.setFileName(saveFilePath+fileName);
    else file.setFileName(saveFilePath+'/'+fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered))
        return;   // remaining do something
    qDebug()<<"init file ok!"<<file.fileName();
}

void UdpReceiver::setSaveFilePath(QString path)
{
    saveFilePath = path;
}

void UdpReceiver::setChatPort(int port)
{
    chatPort = port;
}

void UdpReceiver::readPendingDatagrams()
{

    while(udpSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket.readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        QString strMesg = QString::fromUtf8(datagram);
        if(strMesg[0] == '1' || strMesg[0] == '3' ||
                strMesg[0] == '4' || strMesg[0] == '5'){
            //'1': message  || '3':request for sending file
            //  '4':accept the file or not || '5': broadcast,get online users
            emit resultReady(strMesg);
        }
        else if(strMesg[0] == '2'){
            //process the filedata that have received
            // template: "2#userName#fileName#blockId#percent#blockData"
            //blockData may contains the character '#'
            int pos,num=0;//num is the '#' have counted
            for(pos=0; pos<strMesg.length() && num<5; pos++){
                if(strMesg[pos]=='#')
                    num++;
            }//after the for loop,num=5,pos is the size of "2#...percent#"
            QString strMesgCopy =strMesg;
            QString strHead = strMesg.replace(pos,strMesg.length()-pos,"");
            QString strData = strMesgCopy.replace(0,pos,"");
            //qDebug()<<"receive strHead="<<strHead;
            QByteArray fileDataGram;
            fileDataGram.append(strData);
            file.write(fileDataGram.data(),fileDataGram.size());
            QStringList list = strHead.split('#');
            QString strProgress = "文件 "+list.at(2)+" 正在接收..."+list.at(4)+"%";
            emit resultReady(strProgress);
            if(list.at(4) == "100"){
                file.close();
                QString result = "INFO: 来自 "+list.at(1)+"的文件 "+list.at(2)+" 已接收！";
                emit resultReady(result);
            }
        }// end '2'
    }// end while
}

