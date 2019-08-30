#ifndef FINDDIALOG_H_
#define FINDDIALOG_H_

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QEvent>
#include <QPlainTextEdit>
#include <QPointer>

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    FindDialog(QWidget *parent = 0);
    ~FindDialog();

protected:
    QGroupBox m_radioGrBx;

    QGridLayout m_layout;
    QHBoxLayout m_hbLayout;

    QLabel m_findLbl;
    QLineEdit m_findEdit;
    QPushButton m_findBtn;
    QPushButton m_closeBtn;
    QCheckBox m_matchChkBx;
    QRadioButton m_forwardBtn;
    QRadioButton m_backwardBtn;

    void initControl();
    void connectSlot();

    bool cs;
    bool forward;

signals:
    void findByTarget(QString, bool, bool);

protected slots:    
    void onFindClicked();
    void onCloseClicked();

};

#endif // FINDDIALOG_H_
