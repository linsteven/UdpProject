#ifndef KEYPRESSEATER_H
#define KEYPRESSEATER_H

#include <QObject>
#include<QEvent>

class KeyPressEater : public QObject
{
    Q_OBJECT
public:
    KeyPressEater();
    ~KeyPressEater();
protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void sendEnter();
};

#endif // KEYPRESSEATER_H
