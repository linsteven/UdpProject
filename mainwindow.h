#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include"udpsender.h"
#include"udpreceiver.h"
#include"receivefiledialog.h"
#include"keypresseater.h"
#include <QMainWindow>
#include<QThread>
#include<QCloseEvent>
#include<QTimer>
#include<QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QThread receiveThread;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void doUserName();
    void getMyAdress(QString &myIp, QString &);//local ip, broadcast
    void addTabelItem(QString username, QString userIp, int row);
    void updateOnline(QString username, QString userIP);
    void deleteOnlineTable(QString ip);

private slots:
    void on_btnOpenfile_clicked();
    void handleResults(const QString &);
    void handleProgess(const QString &);
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
    udpSender *sender;
    udpReceiver *receiver;
    receiveFileDialog *reFDialog;
    QString fileSavePath;
    QString userName;  //my name
    QString ip; //聊天对象的ip地址
    QString fileIp;
    QString myIp;
    QString broadcast;// broadcast address

    QString fileName;
    QString filePath;
    KeyPressEater *keyPressEater;
    int chatPort;
    QTimer* timer;//发送在线报文
    QTimer* timerCheck;//检查是否在线
    int onlineCount;
    int defaultTableLines; // equals to 10
    QMap<QString, QDateTime> onlineMap; // < ip , onlineTime >  IP地址，最近接收其在线信息的时间
    QMap<QString,QString> userMap;  //<ip,online username>
};

#endif // MAINWINDOW_H
