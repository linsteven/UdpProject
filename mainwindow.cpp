#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QInputDialog>
#include<QMessageBox>
#include <QDebug>
#include<QTextCodec>
#include<QProcess>
#include<QHostInfo>
#include<QDateTime>
#include<QNetworkInterface>
#include<QList>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    fileName="";
    fileSavePath = "C:/";  //default
    userName = "未知";  //default
    //ip = "127.0.0.1"; //localhost
    sender = new UdpSender();
    chatPort = 7050;   //can be changed
    onlineCount = 0;
    defaultTableLines =10;
    sender->setChatPort(chatPort);
    getMyAdress(myIp,broadcast);
    isLogined = false;

    reFDialog = new receiveFileDialog();
    reFDialog->setWindowTitle("文件接收提醒");
    ui->setupUi(this);

    setWindowTitle("局域网文件UDP传输工具");
    ui->mesgShow->setReadOnly(true);
    ui->chatStatusLabel->setText("<p><font color=red>未选择聊天对象...</font></p>");
    ui->btnOpenfile->setIcon(QIcon(":images/sendFileIcon2.jpg"));
    ui->btnOpenfile->setIconSize(QSize(71,71));//ui->btnOpenfile->width(),ui->btnOpenfile->height()));(QSize(71,71));

    //set the function of input and send file  disabled
    ui->mesgEdit->setDisabled(true);
    ui->btnOpenfile->setDisabled(true);

    //if "enter" or "return " key is pressed,the message will be sent
    keyPressEater = new KeyPressEater();
    connect(keyPressEater,&KeyPressEater::sendEnter,this,&MainWindow::slotSendMesg);
    ui->mesgEdit->installEventFilter(keyPressEater);

    receiver = new UdpReceiver();
    receiver->setChatPort(chatPort);
    connect(receiver , &UdpReceiver::resultReady, this, &MainWindow::handleResults);
    connect(sender,&UdpSender::sendFileProgress,this,&MainWindow::handleProgess);

    connect(ui->setPathAction,&QAction::triggered,this,&MainWindow::slotSetFileSavePath);
    connect(ui->setUserNameAction,&QAction::triggered,this,&MainWindow::slotSetUserName);
    connect(ui->openSavedAction,&QAction::triggered,this,&MainWindow::slotOpenSaved);

    connect(ui->actionCurSavePath,&QAction::triggered,this,&MainWindow::slotSetFileSavePath);
    connect(ui->actionUserName,&QAction::triggered,this,&MainWindow::slotSetUserName);

    connect(reFDialog,&receiveFileDialog::isAccepted,this,&MainWindow::slotReceiveFile);

    doUserName();
    doFilePath();
    receiver->setSaveFilePath(fileSavePath);
    ui->actionUserName->setText("  "+userName); //show the userName on the Menu
    ui->actionCurSavePath->setText("  "+fileSavePath);

    //初始化在线IP列表，并显示在线用户
    //init the list of online ip,and show the online users on the table
    QStringList header;
    header<<"在线用户"<<"IP";
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setRowCount(defaultTableLines); // default is 10
    connect(ui->tableWidget,&QTableWidget::cellClicked,this,&MainWindow::slotSelectRow);

    QHeaderView *headerView = ui->tableWidget->verticalHeader();
    headerView->setHidden(true); //hide the line number
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setAlternatingRowColors(true); //one line grey, next is white
    ui->tableWidget->setFrameShape(QFrame::NoFrame);


    //设置定时器timer，定时广播发出在线UDP报文
    //set the timer to send broadcast UDP packet to inform others my online status
    slotSendOnline();
    timer = new QTimer();
    connect(timer,&QTimer::timeout,this,&MainWindow::slotSendOnline);
    connect(sender,&UdpSender::signalSendOnline,this,&MainWindow::slotSendOnline);
    timer->start(3000); //send online mesg per 5 secs

    //timeCheck is setted to check the users are online or not
    timerCheck = new QTimer();
    connect(timerCheck,&QTimer::timeout,this,&MainWindow::slotCheckOnline);
    timerCheck->start(10000); //check per 10 secs
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doUserName()
{
    //read or set the username
    QFile file("username.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        bool isOK;
        QString name = QInputDialog::getText(NULL, "set your username",
                      "Please set your username",QLineEdit::Normal,
                      "your username", &isOK);
        if(isOK && name.trimmed()!="" && name.trimmed()!="your username"){
            //设置好用户名，并且用户名不为空
            ui->mesgShow->append("<b>INFO:</b> You have set your username as : <font color=grey>"
                                 + name +"</font>\n");
            ui->mesgShow->append("");
            userName = name;
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
                ui->mesgShow->append("store your username failed!");
                ui->mesgShow->append("");
                isLogined = true;
                return;
            }
            QTextStream out(&file);
            out.setCodec(QTextCodec::codecForName("GB2312"));
            out << name;
            file.close();
        }else{
            ui->mesgShow->append("<font color=red><b>NOTICE:</b></font> You haven't set your username. "
                                 "For better experience,please set it later.\n");
            ui->mesgShow->append("");
        }
    }else{
        //read the username from "username.txt"
        QTextStream in(&file);
        in.setCodec(QTextCodec::codecForName("GB2312"));
        //userName = file.readLine();
        in >> userName;
        ui->statusBar->showMessage("Welcome, "+userName,2000);
        file.close();
    }
    isLogined = true;
}

void MainWindow::doFilePath()
{
    //read the saveFilePath
    QFile filePath("savePath.txt");
    if(!filePath.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }else{
        QTextStream in(&filePath);
        in.setCodec(QTextCodec::codecForName("GB2312"));
        in >> fileSavePath;
        receiver->setSaveFilePath(fileSavePath);
        filePath.close();
    }
}

void MainWindow::getMyAdress(QString &myIp,QString &broadcast)
{
    /*
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach(QHostAddress address,info.addresses())
    {
        qDebug() <<"##"<<address.toString();
         if(address.protocol() == QAbstractSocket::IPv4Protocol &&
                 !address.toString().endsWith(".1") &&  //windows
                 !address.toString().endsWith(".2") )  //mac
             //return address.toString();
             qDebug() << address.toString();
    }*/
    bool isFinded = false;
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
        //遍历每一个IP地址(每个包含一个IP地址，一个子网掩码和一个广播地址)
            foreach(QNetworkAddressEntry entry, entryList)
            {
                if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol &&
                        !entry.ip().toString().endsWith(".1") &&  //windows
                        !entry.ip().toString().endsWith(".2") ){  //mac
                    broadcast = entry.broadcast().toString();
                    myIp =  entry.ip().toString();
                    if(broadcast != ""){
                        isFinded = true;
                        break;
                    }
                }
            }
            if(isFinded)break;
    }
}

void MainWindow::on_btnOpenfile_clicked()
{
    QString file = QFileDialog::getOpenFileName(this,"select a file to send","/","");
    if(file == ""){
        ui->mesgShow->append("<font color=red><b>Notice:</b></font>未选择文件，请重新选择要发送的文件...\n");
        ui->mesgShow->append("");
    }else{
        QFileInfo fi = QFileInfo(file);
        filePath = fi.absolutePath();
        fileName = fi.fileName();
        ui->mesgShow->append("<b>INFO：</b> 发送文件申请中...\n");
        ui->mesgShow->append("");
        sender->sendFileRequest(filePath,fileName,userName,ip,myIp);
    }
}

void MainWindow::handleResults(const QString &result)
{
    if(result[0] == '1'){
        //new message text
        //template: "1#userName#mesg"
        QStringList list = result.split('#');
        ui->mesgShow->append("<b><font color=grey>" + list.at(1) + " :</font></b> "
                             + list.at(2)+"\n");
        ui->mesgShow->append("");
    }else if(result[0] == '3'){
        //request for sending file
        //example: "3#userName#fileName#strSize#requestIp"
        QStringList list = result.split('#');
        reFDialog->setInformation(list.at(1),list.at(2),list.at(3));
        ui->mesgShow->append("<b>INFO:</b> <font color=gray>" + list.at(1)+"</font> 请求给你发文件 "+list.at(2)+"\n");
        ui->mesgShow->append("");
        fileIp = list.at(4);
        fileName = list.at(2);
        reFDialog->show();  //show the dialog to let you choose accept the file or not
    }else if(result[0] == '4'){
        //responce for sending file request
        //example: "4#userName#0"
        qDebug()<<result;
        QStringList list = result.split('#');
        if(result.endsWith("0")){
            ui->mesgShow->append("<b>INFO:</b> <font color=grey>" + list.at(1) +"</font> 拒绝接收您的文件！\n");
            ui->mesgShow->append("");
        }else{
            ui->mesgShow->append("<b>INFO</b>: 对方同意，开始发送文件  "+fileName+"...\n");
            ui->mesgShow->append("");
            int status = sender->sendFile(filePath,fileName,userName,ip,myIp,broadcast);
            if(status == -1){
                ui->mesgShow->append("<font color=red><b>ERROR:</b><font> 文件读取失败！再尝试\n");
                ui->mesgShow->append("");
                sender->sendFile(filePath,fileName,userName,ip,myIp,broadcast); //read file failed, try to send again
            }else{
                ui->mesgShow->append("<b>INFO:</b> 文件"+ fileName +" 发送成功！\n");
                ui->mesgShow->append("");
            }
        }
        fileName = "";
        filePath = "";
    }else if(result[0] == '5'){
        if(isLogined){
            QStringList list = result.split('#');
            updateOnline(list.at(1),list.at(2));
        }
    }
    else if(result.startsWith("INFO")){
        ui->mesgShow->append(result);
        ui->mesgShow->append("");
    }else if(result.startsWith("文件")){
        ui->statusBar->showMessage(result,100);
    }

}

void MainWindow::handleProgess(const QString &mesg)
{
    ui->statusBar->showMessage(mesg,100);
}

void MainWindow::slotSendMesg()
{
    QString str = ui->mesgEdit->toPlainText();
    if( str!= ""){
        sender->sendMesg(str,userName,ip);
        ui->mesgShow->append("<b><font color=grey>" + userName + ":</font></b>"+ str+"\n");
        ui->mesgShow->append("");
        ui->mesgEdit->setText("");
        ui->mesgEdit->setFocus();
    }
}

void MainWindow::slotSetFileSavePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                  "/",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(dir != ""){
        fileSavePath = dir;
        //reFDialog->setSavePath(fileSavePath);
        receiver->setSaveFilePath(fileSavePath);
        QFile file("savePath.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            ui->mesgShow->append("store the new savePath failed!");
            ui->mesgShow->append("");
            return;
        }
        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("GB2312"));
        out << dir;
        file.close();
        ui->mesgShow->append("<b>INFO:</b>成功设置接收文件保存目录为：" + dir+"\n");
        ui->mesgShow->append("");
        ui->actionCurSavePath->setText("  "+fileSavePath);
    }
}

void MainWindow::slotSetUserName()
{
    bool isOK;
    QString newName = QInputDialog::getText(NULL, "new username",
                        "New username",QLineEdit::Normal,
                        "", &isOK);
    if(isOK && newName.trimmed() != ""){
        QFile file("username.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            ui->mesgShow->append("<b>EORROR:</b> Update your username failed!");
            ui->mesgShow->append("");

            return;
        }
        userName = newName;
        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("GB2312"));
        out << newName;
        file.close();
        ui->mesgShow->append("<b>INFO:</b> Your username has been updated! "
                             "The new username is : <font color=grey>" + newName + "</font>\n");

        ui->mesgShow->append("");ui->actionUserName->setText("  "+userName);
    }else{
        ui->mesgShow->append("<font color=red><b>Notice:</b></font> You haven't updated your username\n");
        ui->mesgShow->append("");
    }

}

void MainWindow::slotReceiveFile(bool isAccepted)
{
    sender->sendFileResponse(isAccepted,userName,fileIp);
    if(isAccepted){
        receiver->initFile(fileName);
        ui->mesgShow->append("<b>INFO:</b> 已同意接收文件！");
        ui->mesgShow->append("");
    }else{
        ui->mesgShow->append("<b>INFO:</b> 已拒绝接收文件！");
        ui->mesgShow->append("");
    }
}

void MainWindow::slotOpenSaved()
{
    QString path = fileSavePath;
    path.replace("/","\\");
    QProcess::execute("explorer " + path);
}

void MainWindow::slotSelectRow(int row, int) //(row,column)  ,column isn't needed
{
    QTableWidgetItem *userItem = ui->tableWidget->item(row,0);
    QTableWidgetItem *ipItem = ui->tableWidget->item(row,1);
    bool isChosen = true;
    if(userItem != NULL ){
        if(userItem->text() != userName){
            //更新聊天对象
            ip = ipItem->text();
            ui->chatStatusLabel->setText("<p>当前正在与 <font color=grey>"
                                         + userItem->text() + "</font> 聊天...</p>");
            ui->mesgEdit->setDisabled(false);
            ui->btnOpenfile->setDisabled(false);
        }else{
            ui->chatStatusLabel->setText("<p><font color=red>暂不支持自娱自乐，请多与他人沟通...</font></p>");
            isChosen = false;
            //ip = ipItem->text();  //debug for sending message or file to localhost, and the ip should be setted as "127.0.0.1"
            //ui->mesgEdit->setDisabled(false);
            //ui->btnOpenfile->setDisabled(false);
        }
    }else{

        ui->chatStatusLabel->setText("<p><font color=red>未选择聊天对象...</font></p>");
        isChosen = false;
    }
    if(!isChosen){
        //未选择聊天对象, ip清空，输入框、发文件按钮 设为disabled
        ip = "";
        ui->mesgEdit->setDisabled(true);
        ui->btnOpenfile->setDisabled(true);
    }
}

void MainWindow::slotSendOnline()
{
    sender->sendOnline(userName,myIp,broadcast);
}

void MainWindow::slotCheckOnline()
{
    QDateTime curTime = QDateTime::currentDateTime();
    QMap<QString, QDateTime>::iterator i = onlineMap.begin();
    for (i = onlineMap.begin(); i != onlineMap.end(); ){
        QDateTime onlineTime = i.value();
        uint secs = onlineTime.time().secsTo(curTime.time());
        if(secs > 10){//offline
            deleteOnlineTable(i.key());
            //QMap<QString, QDateTime>::iterator it = i;
            i = onlineMap.erase(i);
        }else{
            ++i;
        }
    }//end for
}

void MainWindow::addTabelItem(QString username, QString userIp, int row)
{
    QTableWidgetItem *item1 = new QTableWidgetItem(username,0);
    QTableWidgetItem *item2 = new QTableWidgetItem(userIp,0);
    item1->setTextAlignment(Qt::AlignCenter);
    item2->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(row,0,item1);
    ui->tableWidget->setItem(row,1,item2);
}

void MainWindow::updateOnline(QString username, QString userIP)
{
    QDateTime curTime = QDateTime::currentDateTime();
    bool isEmpty = ui->tableWidget->findItems(userIP,Qt::MatchExactly).isEmpty();
    //qDebug()<<username<<" "<<userIP;
    if(isEmpty){ //该用户当前不在线,加入online list
        addTabelItem(username,userIP,onlineCount);
        onlineCount++;
        if(onlineCount>=defaultTableLines)  //>=10 减少时，删除row
            ui->tableWidget->insertRow(onlineCount);
        //add <ip,onlineTime(curTime)> to the onlineMap
        onlineMap.insert(userIP,curTime);
        userMap.insert(userIP,username);
        if(username != userName)
            ui->statusBar->showMessage(username+" 上线了!",1000);
    }else{
        //updata the user's onlineTime
        if(onlineMap.contains(userIP)){ //if may be unnecessary
            onlineMap[userIP] = curTime;
        }
        //有用户更改用户名，需更新用户名
        if(userMap[userIP] != username){
            userMap[userIP] = username;
            for(int i=0;i<onlineCount;i++){
                if(ui->tableWidget->item(i,1)->text() == userIP){
                    QString oldname = ui->tableWidget->item(i,0)->text();
                    ui->tableWidget->item(i,0)->setText(username);
                    if(username != userName){
                        ui->mesgShow->append("<p><b>INFO:</b> 用户 <font color=grey>"+oldname
                                             +"</font> 改名为 <font color=grey>"+username
                                             +"\n</font></p>");
                        ui->mesgShow->append("");
                    }
                    break;
                }
            }
        }// end
    }//end else ,user is online
}

void MainWindow::deleteOnlineTable(QString ip)
{
    int row;
    bool flag = false;
    QString userName = "";
    for(row = 0; row < onlineCount; row++){
        QTableWidgetItem *ipItem = ui->tableWidget->item(row,1);
        if(ipItem->text() == ip){
            userName = ui->tableWidget->item(row,0)->text();
            ui->tableWidget->removeRow(row);//delete the row
            flag = true;
            break;
        }
    }
    if(flag){
        if(onlineCount < defaultTableLines)
            ui->tableWidget->insertRow(defaultTableLines-1);//插在最后一行
        onlineCount--;
        ui->statusBar->showMessage(userName+" 下线了!",2000);
    }

    /*
    for(row = 0; row < onlineCount; row++){
        QTableWidgetItem *ipItem = ui->tableWidget->item(row,1);
        if(ipItem->text() == ip){
            //delete the row
            //QTableWidgetItem *userItem = ui->tableWidget->item(row,0);
            //ipItem->setText("");
            //userItem->setText("");
            flag = true;
            break;
        }
    }
    if(flag){
        if(row < onlineCount-1){ //删除行在中间，需要后面 行 往前移动
            for(int i=row; i<onlineCount-1;i++){
                QTableWidgetItem *userItem1 = ui->tableWidget->item(i,0);
                QTableWidgetItem *ipItem1 = ui->tableWidget->item(i,1);
                QTableWidgetItem *userItem2 = ui->tableWidget->item(i+1,0);
                QTableWidgetItem *ipItem2 = ui->tableWidget->item(i+1,1);
                userItem1->setText(userItem2->text());
                ipItem1->setText(ipItem2->text());
                }
        }
        //清空最后一行
        ui->tableWidget->item(onlineCount-1,0)->setText("");
        ui->tableWidget->item(onlineCount-1,1)->setText("");
        if(onlineCount > defaultTableLines){
            //删除最后行，保持默认10行
            ui->tableWidget->removeRow(onlineCount);  //写完发现有这个函数，泪崩，，几十行白写了
        }
        onlineCount--;
    }
    */
}

