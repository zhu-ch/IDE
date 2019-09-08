#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QString>
#include <QDebug>
#include <vector>

class DebugDialog : public QDialog
{
public:
    DebugDialog();
    void setProgram(QString);
    void setBreakpoints(std::vector<int>);
    void showProperties();

private slots:
    void slotNext();
    void slotStep();
    void slotContinue();
    void slotRun();
    void slotQuit();
    void slotAddVar();

private:
    QGridLayout *layout;
    QVBoxLayout *vbLayout;

    QPushButton *nextBtn;//单步跳过
    QPushButton *stepBtn;//单步进入
    QPushButton *continueBtn;//继续执行
    QPushButton *runBtn;//开始调试
    QPushButton *quitBtn;//退出
    QPushButton *addVarBtn;//添加变量
    QLineEdit *inputVar;//变量输入
    QTextEdit *printArea;//输出部分，只读

    void initWIdgets();

    QString program;
    std::vector<int> breakpoints;
};

#endif // DEBUGDIALOG_H
