#ifndef REPACEDIALOG_H
#define REPACEDIALOG_H

#include "FindDialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QPlainTextEdit>

class ReplaceDialog : public FindDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = 0);

protected:
    QLabel m_replaceLbl;
    QLineEdit m_replaceEdit;
    QPushButton m_replaceBtn;
    QPushButton m_replaceAllBtn;

    void initControl();
    void connectSlot();

protected slots:
    void onReplaceClicked();
    void onReplaceAllClicked();

signals:
    void replaceSelect(QString, QString, bool, bool, bool);

};
#endif // REPACEDIALOG_H
