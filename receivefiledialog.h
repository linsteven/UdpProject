#ifndef RECEIVEFILEDIALOG_H
#define RECEIVEFILEDIALOG_H

#include <QDialog>

namespace Ui {
class receiveFileDialog;
}

class receiveFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit receiveFileDialog(QWidget *parent = 0);
    ~receiveFileDialog();
    void setInformation(QString from, QString fileName, QString fileSize);
    void setSavePath(QString strPath);

signals:
    void isAccepted(bool);

private slots:
    void on_acceptButton_clicked();

    void on_refuseButton_clicked();

private:
    Ui::receiveFileDialog *ui;
    QString savePath;
};

#endif // RECEIVEFILEDIALOG_H
