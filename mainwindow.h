#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include"udpsender.h"
#include"udpreceiver.h"
#include"receivefiledialog.h"
#include"keypresseater.h"
#include <QMainWindow>
#include<QCloseEvent>
#include<QTimer>
#include<QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void doUserName();  //read or set the username
    void doFilePath();  //read the saveFilePath
    void getMyAdress(QString &myIp, QString &);//local ip, broadcast address
    void addTabelItem(QString username, QString userIp, int row);
    void updateOnline(QString username, QString userIP);
    void deleteOnlineTable(QString ip);

private slots:
    void on_btnOpenfile_clicked();
    void handleResults(const QString &);
    void handleProgess(const QString &); //show progress on the statusBar
    void slotSendMesg();
    void slotSetFileSavePath();
    void slotSetUserName();
    void slotReceiveFile(bool);
    void slotOpenSaved();
    void slotSelectRow(int,int);
    void slotSendOnline();
    void slotCheckOnline();


private:
    Ui::MainWindow *ui;
    UdpSender *sender;
    UdpReceiver *receiver;
    receiveFileDialog *reFDialog;
    QString fileSavePath; //where to store received files
    QString userName;  //my name
    QString ip; //ip address of chatter
    QString fileIp; // the ip of the one that want to send file to me, it's needed when response
    QString myIp;
    QString broadcast;// broadcast address

    QString fileName;  // file: the file to be sent
    QString filePath;
    KeyPressEater *keyPressEater;
    int chatPort;
    QTimer* timer;//send online packet
    QTimer* timerCheck;//check is online or not
    int onlineCount; //the lines of online tabel
    int defaultTableLines; // equals to 10
    QMap<QString, QDateTime> onlineMap; // < ip , onlineTime >  IP地址，最近接收其在线信息的时间
    QMap<QString,QString> userMap;  //<ip,online username>

    bool isLogined; //the manwindow is showed,then set it true
};

#endif // MAINWINDOW_H
