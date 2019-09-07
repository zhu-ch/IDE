#include "runthread.h"
#include <QDebug>

RunThread::RunThread(QString s)
{
    this->cmd = s;
}


void RunThread::run(){
    qDebug()<<cmd;
    system(cmd.toStdString().data());
}
