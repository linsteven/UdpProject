#include "keypresseater.h"
#include<QKeyEvent>
KeyPressEater::KeyPressEater()
{

}

KeyPressEater::~KeyPressEater()
{

}

bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            //qDebug("Ate key press %d", keyEvent->key());
            if(keyEvent->key()==Qt::Key_Enter||keyEvent->key()==Qt::Key_Return){
                if((keyEvent->modifiers()&Qt::ShiftModifier)||
                        (keyEvent->modifiers()&Qt::ControlModifier)){
                    //do nothing ,just change line
                    //qDebug("ctrl or shift + enter");
                    //return QObject::eventFilter(obj, event);
                }else{
                    emit sendEnter();
                    return true;
                }
            }//if press enter
            return QObject::eventFilter(obj, event);
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

