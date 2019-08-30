#include "FindDialog.h"
#include <QMessageBox>
#include <QDebug>

FindDialog::FindDialog(QWidget *parent) : QDialog(parent,Qt::WindowCloseButtonHint | Qt::Drawer)
{
    initControl();
    connectSlot();

    setLayout(&m_layout);
    setWindowTitle("Find");

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}
void FindDialog::initControl()
{
    m_findLbl.setText("Find what:");
    m_findBtn.setText("Find Next");
    m_closeBtn.setText("Close");
    m_matchChkBx.setText("大小写匹配");
    //这里的前后是反的！
    m_backwardBtn.setText("向前");
    m_forwardBtn.setText("向后");
    m_forwardBtn.setChecked(true);
    m_radioGrBx.setTitle("查找方向");

    m_radioGrBx.setLayout(&m_hbLayout);  

    m_layout.setSpacing(10);
    m_layout.addWidget(&m_findLbl,0,0);
    m_layout.addWidget(&m_findEdit,0,1);
    m_layout.addWidget(&m_findBtn,0,2);
    m_layout.addWidget(&m_matchChkBx,1,0);
    m_layout.addWidget(&m_radioGrBx,1,1);
    m_layout.addWidget(&m_closeBtn,1,2);
    m_hbLayout.addWidget(&m_forwardBtn);
    m_hbLayout.addWidget(&m_backwardBtn);

}

void FindDialog::connectSlot()
{
    connect(&m_findBtn,SIGNAL(clicked()),this,SLOT(onFindClicked()));
    connect(&m_closeBtn,SIGNAL(clicked()),this,SLOT(onCloseClicked()));
}

void FindDialog::onFindClicked()
{
    QString target = m_findEdit.text();
    cs = m_matchChkBx.isChecked();
    forward = m_forwardBtn.isChecked();
    emit findByTarget(target, cs, forward);
}

void FindDialog::onCloseClicked()
{
    close();
}

FindDialog::~FindDialog()
{

}
