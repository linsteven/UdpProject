#include "receivefiledialog.h"
#include "ui_receivefiledialog.h"

receiveFileDialog::receiveFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::receiveFileDialog)
{
    ui->setupUi(this);
    ui->fromLabel->setText("");
    ui->fileNameLabel->setText("");
    ui->sizeLabel->setText("");
    ui->fileNameLabel->setWordWrap(true);
    //savePath = "";
}

receiveFileDialog::~receiveFileDialog()
{
    delete ui;
}

void receiveFileDialog::setInformation(QString from, QString fileName, QString fileSize)
{
    ui->fromLabel->setText(from);
    ui->fileNameLabel->setText(fileName);
    ui->sizeLabel->setText(fileSize);
}

void receiveFileDialog::setSavePath(QString strPath)
{
    //用于 ”另存为“，暂且未加该功能
    savePath = strPath;
}

void receiveFileDialog::on_acceptButton_clicked()
{
    emit isAccepted(true);
    accept();
}



void receiveFileDialog::on_refuseButton_clicked()
{
    emit isAccepted(false);
    reject();
}
