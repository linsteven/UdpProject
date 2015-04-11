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
    fileSavePath = "C:/";
    userName = "未知";
    //ip = "127.0.0.1";//"124.16.103.254";  //本机
    sender = new udpSender();
    chatPort = 7050;   //可以自定义
    onlineCount = 0;
    defaultTableLines =10;
    sender->setChatPort(chatPort);
    getMyAdress(myIp,broadcast);

    reFDialog = new receiveFileDialog();
    reFDialog->setWindowTitle("文件接收提醒");
    //reFDialog->setSavePath(fileSavePath);


    ui->setupUi(this);

    setWindowTitle("局域网文件UDP传输工具");
    ui->mesgShow->setReadOnly(true);
    //ui->statusBar->setFont(QFont(QFont::Times));
    ui->chatStatusLabel->setText("<p><font color=red>未选择聊天对象...</font></p>");
    ui->btnOpenfile->setIcon(QIcon(":images/sendFileIcon2.jpg"));
    //ui->btnOpenfile->setAutoFillBackground(true);
    ui->btnOpenfile->setIconSize(QSize(71,71));//ui->btnOpenfile->width(),ui->btnOpenfile->height()));(QSize(71,71));
    //ip = "127.0.0.1";
    ui->mesgEdit->setDisabled(true);
    ui->btnOpenfile->setDisabled(true);

    //ui->btnOpenfile->set
    //ui->btnOpenfile->setIconSize(QSize(ui->btnOpenfile->width(),ui->btnOpenfile->height()));

    keyPressEater = new KeyPressEater();
    connect(keyPressEater,&KeyPressEater::sendEnter,this,&MainWindow::slotSendMesg);
    ui->mesgEdit->installEventFilter(keyPressEater);
    receiver = new udpReceiver();
    receiver->setChatPort(chatPort);
    connect(receiver , &udpReceiver::resultReady, this, &MainWindow::handleResults);
    connect(sender,&udpSender::sendFileProgress,this,&MainWindow::handleProgess);

    connect(ui->setPathAction,&QAction::triggered,this,&MainWindow::slotSetFileSavePath);
    connect(ui->setUserNameAction,&QAction::triggered,this,&MainWindow::slotSetUserName);
    connect(ui->openSavedAction,&QAction::triggered,this,&MainWindow::slotOpenSaved);

    connect(ui->actionCurSavePath,&QAction::triggered,this,&MainWindow::slotSetFileSavePath);
    connect(ui->actionUserName,&QAction::triggered,this,&MainWindow::slotSetUserName);

    connect(reFDialog,&receiveFileDialog::isAccepted,this,&MainWindow::slotReceiveFile);

    doUserName();
    receiver->setSaveFilePath(fileSavePath);
    //reFDialog->show();

    //初始化在线IP列表，并显示在线用户
    QStringList header;
    header<<"在线用户"<<"IP";
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(header);
    connect(ui->tableWidget,&QTableWidget::cellClicked,this,&MainWindow::slotSelectRow);

    ui->tableWidget->setRowCount(defaultTableLines); // default is 10

    //隐藏行号
    QHeaderView *headerView = ui->tableWidget->verticalHeader();
    headerView->setHidden(true);
    //headerView->setTextElideMode(Qt::ElideRight);
    //设置自动选择一行
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置只能选择一行，即只能与一个人聊天
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    //设置每行内容不可编辑
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //设置每行一变色，一灰一白
    ui->tableWidget->setAlternatingRowColors(true);
    //ui->tableWidget->verticalHeader()->setVisible(false);//隐藏左边垂直
    ui->tableWidget->setFrameShape(QFrame::NoFrame); //设置无边框
    //ui->tableWidget->setShowGrid(false); //设置不显示格子线
    //columnHeaderItem0->setFont(QFont("Helvetica")); //设置字体
     //获得水平方向表头的Item对象
    //QTableWidgetItem *columnHeaderItem0 = ui->tableWidget->horizontalHeaderItem(3);
    //qDebug()<<columnHeaderItem0->row()<<" = "<<columnHeaderItem0->column();
    //columnHeaderItem0->setBackgroundColor(QColor(0,60,10)); //设置单元格背景颜色


    //设置定时器timer，定时广播发出在线UDP报文
    slotSendOnline();
    timer = new QTimer();
    connect(timer,&QTimer::timeout,this,&MainWindow::slotSendOnline);
    connect(sender,&udpSender::signalSendOnline,this,&MainWindow::slotSendOnline);
    timer->start(3000); //send online mesg per 5 secs
    timerCheck = new QTimer();
    connect(timerCheck,&QTimer::timeout,this,&MainWindow::slotCheckOnline);
    timerCheck->start(9999); //check the users are online or not per 10 secs

    //设置状态栏
   /*
    //背景色设置
    ui->statusBar->setAutoFillBackground(true);
    QPalette p = ui->statusBar->palette();
    p.setColor(QPalette::Window,QColor("#DADADA"));
    ui->statusBar->setPalette(p);*/
    //添加控件
    //QPushButton *btnMesg = new QPushButton();
    //btnMesg->setText("btn");
    //btnMesg->setIcon(QIcon(":/images/mesgIcon.gif"));

    //ui->statusBar->addWidget(btnMesg);
    //btnMesg->setIconSize(QSize(18,18));
    //btnMesg->show();
    ui->actionUserName->setText("  "+userName);
    ui->actionCurSavePath->setText("  "+fileSavePath);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doUserName()  //程序启动时
{
    //read or set the username
    // have added read the saveFilePath
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
                    //qDebug() << "IP Address:" << entry.ip().toString();
                    //qDebug() << "Netmask:" << entry.netmask().toString();//子网掩码
                    //qDebug() << "Broadcast:" << entry.broadcast().toString();//广播地址
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
    //QStringList fileNamesList ;//= new QStringList();
    QString file = QFileDialog::getOpenFileName(this,"select a file to send","/","");
    if(file == ""){
        ui->mesgShow->append("<font color=red><b>Notice:</b></font>未选择文件，请重新选择要发送的文件...\n");
        ui->mesgShow->append("");
    }else{
        QFileInfo fi = QFileInfo(file);
        filePath = fi.absolutePath();
        fileName = fi.fileName();
        //ui->mesgShow->append("INFO：将要发送文件"+ fileName+"\n");
        ui->mesgShow->append("<b>INFO：</b> 发送文件申请中...\n");
        ui->mesgShow->append("");
        sender->sendFileRequest(filePath,fileName,userName,ip,myIp);
    }
}

void MainWindow::handleResults(const QString &result)
{
    if(result[0] == '1'){
        //new message text
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
        reFDialog->show();
    }else if(result[0] == '4'){
        //responce for sending file request
        //example: "4#userName#0"
        qDebug()<<result;
        QStringList list = result.split('#');
        if(result.endsWith("0")){
            ui->mesgShow->append("<b>INFO:</b> <font color=grey>" + list.at(1) +"</font> 拒绝接收您的文件！\n");
            ui->mesgShow->append("");
            fileName = ""; //clear is good ??
            filePath = "";
        }else{
            ui->mesgShow->append("<b>INFO</b>: 对方同意，开始发送文件  "+fileName+"...\n");
            ui->mesgShow->append("");
            //list.at(2)=='1'  can send the file
            //QDateTime curTime1 = QDateTime::currentDateTime();
            //qDebug()<<"start"<<curTime1;
            int status = sender->sendFile(filePath,fileName,userName,ip,myIp,broadcast);
            //QDateTime curTime2 = QDateTime::currentDateTime();
            //qDebug()<<"end:"<<curTime2;
            //qDebug()<<curTime1.time().secsTo(curTime2.time()); 测试发送文件时间
            if(status == -1){
                ui->mesgShow->append("<font color=red><b>ERROR:</b><font> 文件读取失败！再尝试\n");
                ui->mesgShow->append("");
                sender->sendFile(filePath,fileName,userName,ip,myIp,broadcast);
            }else{
                ui->mesgShow->append("<b>INFO:</b> 文件"+ fileName +" 发送成功！\n");
                ui->mesgShow->append("");
            }
            //else{
                fileName = ""; //发送完毕，清空
                filePath = "";
            //}
        }
    }else if(result[0] == '5'){
        //qDebug()<<"get online: "<<result;
        QStringList list = result.split('#');
        updateOnline(list.at(1),list.at(2));
    }
    else if(result.startsWith("INFO")){
        ui->mesgShow->append(result);
        ui->mesgShow->append("");
    }else if(result.startsWith("文件")){
        //progress
        //qDebug()<<"show";
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

void MainWindow::slotSelectRow(int row, int column)
{
    //qDebug()<<"row="<<row<<" col="<<column;
    QTableWidgetItem *userItem = ui->tableWidget->item(row,0);
    QTableWidgetItem *ipItem = ui->tableWidget->item(row,1);
    bool isChosen = true;
    if(userItem != NULL ){
        if(userItem->text() != userName){
            //更新聊天对象
            ip = ipItem->text();
            //ui->chatStatusLabel->setText("<h4><i>Hello</i><font color=red>Qt!</font></h4>");
            ui->chatStatusLabel->setText("<p>当前正在与 <font color=grey>"
                                         + userItem->text() + "</font> 聊天...</p>");
            ui->mesgEdit->setDisabled(false);
            ui->btnOpenfile->setDisabled(false);
        }else{
            ui->chatStatusLabel->setText("<p><font color=red>暂不支持自娱自乐，请多与他人沟通...</font></p>");
            isChosen = false;
            //ip = ipItem->text();  //单机测试用
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
    //QString myIp,broadcast;
    //getMyAdress(myIp,broadcast);
    //qDebug()<<"broadcast:"<<broadcast;
    sender->sendOnline(userName,myIp,broadcast);
}

void MainWindow::slotCheckOnline()
{
    QDateTime curTime = QDateTime::currentDateTime();
    QMap<QString, QDateTime>::iterator i = onlineMap.begin();
    for (i = onlineMap.begin(); i != onlineMap.end(); ){
        //qDebug()<< i.key() << ": " << i.value() ;
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
    //ui->tableWidget->setRowCount();
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
        //qDebug()<<"add:"<<onlineCount;   发现待修复bug
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
            //qDebug()<<"update onlineTime:"<<onlineMap[userIP].toString(Qt::TextDate);
            //qDebug()<<onlineMap.size();
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
    //onlineCo
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

