#ifndef MYQTREEVIEW_H
#define MYQTREEVIEW_H

#include <QTreeView>
#include <QList>
#include <QStandardItem>
#include <QMouseEvent>
#include <QDirModel>

class MyQTreeView : public QTreeView
{
    Q_OBJECT
public:
//    explicit TreeView(QWidget *parent = 0);
//    void iteratorOverItems();
//    QList<QStandardItem *> returnTheItems();


    //void mouseDoubleClickEvent(QMouseEvent *event);
    QString selectFilePath;
    QDirModel *model;


signals:
    //void doubleClicked(const QModelIndex &index);

public slots:


private:
    //QStandardItemModel *model;
};


#endif // MYQTREEVIEW_H
