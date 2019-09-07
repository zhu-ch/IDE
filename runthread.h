#ifndef RUNTHREAD_H
#define RUNTHREAD_H

#include <QThread>
#include <QString>

class RunThread : public QThread
{
    Q_OBJECT
public:
    RunThread(QString);

private:
    QString cmd;
    void run() Q_DECL_OVERRIDE;
};

#endif // RUNTHREAD_H
