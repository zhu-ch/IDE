#include "ReplaceDialog.h"
#include <QDebug>

ReplaceDialog::ReplaceDialog(QWidget *parent):FindDialog(parent)
{
    initControl();
    connectSlot();

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

void ReplaceDialog::initControl()
{
    setWindowTitle("Replace");

    m_replaceLbl.setText("Replace To:");
    m_replaceBtn.setText("Replace");
    m_replaceAllBtn.setText("Replace All");

    m_layout.removeWidget(&m_matchChkBx);
    m_layout.removeWidget(&m_radioGrBx);
    m_layout.removeWidget(&m_closeBtn);

    m_layout.addWidget(&m_replaceLbl,1,0);
    m_layout.addWidget(&m_replaceEdit,1,1);
    m_layout.addWidget(&m_replaceBtn,1,2);
    m_layout.addWidget(&m_matchChkBx,2,0);
    m_layout.addWidget(&m_radioGrBx,2,1);
    m_layout.addWidget(&m_replaceAllBtn,2,2);
    m_layout.addWidget(&m_closeBtn,3,2);

}

void ReplaceDialog::connectSlot()
{
    connect(&m_replaceBtn,SIGNAL(clicked()),this,SLOT(onReplaceClicked()));
    connect(&m_replaceAllBtn,SIGNAL(clicked()),this,SLOT(onReplaceAllClicked()));
}

void ReplaceDialog::onReplaceClicked()
{
    QString target = m_findEdit.text();
    QString to = m_replaceEdit.text();
    cs = m_matchChkBx.isChecked();
    forward = m_forwardBtn.isChecked();

    emit replaceSelect(target, to, cs, forward, false);
}

void ReplaceDialog::onReplaceAllClicked()
{
    QString target = m_findEdit.text();
    QString to = m_replaceEdit.text();

    emit replaceSelect(target, to, cs, forward, true);
}
