#include "myqtreeview.h"
#include <QModelIndex>
#include <QDebug>

// 鼠标双击事件(输出被双击item的文本)
//void MyQTreeView::mouseDoubleClickEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton)
//    {
//        selectFilePath="";
//        QModelIndex index = currentIndex();
//        //TODO 获取绝对路径
//        //qDebug()<<index.data().toString();//文件名
//        selectFilePath = QString(index.data().toString()) + selectFilePath;
//        while(index.parent().data().toString()!=""){
//            index=index.parent();
//            //qDebug()<<index.data().toString();
//            selectFilePath = QString(index.data().toString()) + QString("/") + selectFilePath;
//        }
//        qDebug()<<"+++";
//        qDebug()<<selectFilePath;
//    }
//}
