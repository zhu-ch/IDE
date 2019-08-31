/****************************************************************************
**
** Copyright (C) 2004-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Qsci/qscilexercpp.h>
#include <QDialog>
#include "finddialog.h"
#include "replacedialog.h"

class QAction;
class QMenu;
class QsciScintilla;

class QscilexerCppAttach : public QsciLexerCPP
{
    Q_OBJECT

public:
    const char *keywords(int set) const;
};

class MyKeyPressEater : public QObject
{
    Q_OBJECT

signals:
    void keyPressSiganl_braceComplete(int);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);
    void wheelEvent(QWheelEvent *event);

public slots:
    void newFile();
    void open();
    void openfolder();
    bool save();
    bool saveAs();
    void mycompile();

    void about();
    void documentWasModified();
    void on_margin_clicked(int, int, Qt::KeyboardModifiers);
    void do_cursorChanged();

    void showReplace();
    void showFind();
    void handleFindByTarget(QString, bool, bool);
    void handleReplaceSelect(QString, QString, bool, bool, bool);

    void change_name();//F2
    void chang_all_name();//change

    void showColor();
    void showFont();

    void handleBraceComplete(int);

private:
    void bindSignals();
    void setTextEdit();                             //代码编辑区
    void initLogtext();                             //编辑信息提示区域
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool LoadLogFile(const QString &fileName);      //读取编译信息文件
    bool saveFile(const QString &fileName);
    void setCurrentFileName(const QString &fileName);
    //QString strippedName(const QString &fullFileName);

    QsciScintilla *textEdit;            //代码编辑框
    //QFrame *colorFrame;                 //样式 大小
    //QLineEdit *fontLineEdit;            //颜色
    //QscilexerCppAttach *textLexer;
    QsciLexer *textLexer;               //语法分析器
    QTextEdit *LogText;                 //build log text
    QString curFile;                    //当前打开文件的路径

    /***菜单栏部分***/

    //文件
    QMenu *fileMenu;
    QAction *newAct;            //新建
    QAction *openAct;           //打开
    QAction *openfolderAct;     //打开文件夹
    QAction *saveAct;           //保存
    QAction *saveAsAct;//另存为
    QAction *exitAct;//退出

    //编辑
    QMenu *editMenu;
    QAction *cutAct;//剪切
    QAction *copyAct;//复制
    QAction *pasteAct;//粘贴
    QAction *selectallAct;//全选
    QAction *undoAct;//撤销
    QAction *redoAct;//重做
    QAction *replaceAct;//替换
    QAction *findAct;//查找
    QAction *changeAct;//更改变量

    //帮助
    QMenu *helpMenu;
    QAction *aboutAct;
    QAction *aboutQtAct;


    //编译运行
    QMenu *compileMenu;
    QAction *compileAct;//编译
    QAction *runAct;//运行

    //格式
    QMenu *formMenu;
    QAction *fontAct;//字体大小和样式
    QAction *colorAct;//字体颜色

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *compileToolBar;
    QToolBar *formToolBar;

    //查找和替换窗口
    FindDialog findDialog;
    ReplaceDialog replaceDialog;

    //QgridLayout
    QVBoxLayout *mainLayout;    //格子布局

    //变量重命名相关：因为没法实现匿名函数所以需要的变量
    QString variableName;
    QLineEdit *lineEdit;

    //监听按键
    MyKeyPressEater *keyPressEater;

    //光标位置
    int cursorLine;
    int cursorIndex;
};

#endif
