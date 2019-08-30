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

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>
#include <QDebug>

//QsciScintilla本体
#include<Qsci/qsciscintilla.h>
//Lua语言的词法分析器
#include <Qsci/qscilexerlua.h>
//自动补全的apis
#include <Qsci/qsciapis.h>

#include "mainwindow.h"
#include"replacedialog.h"

//可自定义关键字 ok
const char * QscilexerCppAttach::keywords(int set) const
{
    if(set == 1 || set == 3)
        return QsciLexerCPP::keywords(set);
    if(set == 2)
        return
        //可自定义关键字
        "code or die bit BIT";

    return 0;
}
/****讲解信号SINGAL与槽SLOT的机制connect****/
/*
 * 简单来说就是点击和函数进行绑定
 * 不过有的是有也不一定是点击 反正就是二者可以绑定
 * 具体参考： http://c.biancheng.net/view/1823.html
*/
MainWindow::MainWindow()
{
    textEdit = new QsciScintilla;
    setCentralWidget(textEdit);

    keywordDeal();//自定义设置
    createActions();//点击按钮与函数进行绑定
    createMenus();//菜单栏
    createToolBars();//工具栏
    createStatusBar();//状态栏

    readSettings();

    connect(textEdit, SIGNAL(textChanged()),this, SLOT(documentWasModified()));//点击断点区域 绑定有关断点的函数

    setCurrentFileName("");

    //处理查找、替换窗口的信号
    connect(&findDialog, SIGNAL(findByTarget(QString, bool, bool)), this, SLOT(handleFindByTarget(QString, bool, bool)));
    connect(&replaceDialog, SIGNAL(findByTarget(QString, bool, bool)), this, SLOT(handleFindByTarget(QString, bool, bool)));
    connect(&replaceDialog, SIGNAL(replaceSelect(QString, QString, bool, bool, bool)),
            this, SLOT(handleReplaceSelect(QString, QString, bool, bool, bool)));
}
//关闭窗口事件
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {//当关闭时 提示是否保存后 用户选择保存（并成功）或不保存
        writeSettings();
        event->accept();
    } else {//当关闭时 提示是否保存后 用户选择取消
        event->ignore();
    }
}

//新建 ok
void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName("");
    }
}
//打开 ok
void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}
//保存 ok
bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}
//另存为 ok
bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

//关于 ok
void MainWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}

void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->isModified());
}
//点击断点区域后执行的函数
void MainWindow::on_margin_clicked(int margin, int line, Qt::KeyboardModifiers state)
{
    Q_UNUSED(state);

    if (margin == 1) {//当前位置有断点 去掉
        if (textEdit->markersAtLine(line) != 0) {
            textEdit->markerDelete(line, 1);
//            做一些去掉断点的逻辑程序
        } else {//当前位置无断点 添加
            textEdit->markerAdd(line, 1);
//            做一些增加断点的逻辑程序
        }
    }
}
//自定义设置部分
void MainWindow::keywordDeal()
{
    textLexer = new QscilexerCppAttach;                                             //绑定Cpp的关键字
    textLexer->setColor(QColor(Qt:: green),QsciLexerCPP::CommentLine);              //设置自带的注释行为绿色
    textLexer->setColor(QColor(Qt:: yellow),QsciLexerCPP::KeywordSet2);             //设置自定义关键字的颜色为黄色
    textEdit->setLexer(textLexer);

    //1. 设置自动补全的字符串和补全方式 ok
    QsciAPIs *apis = new QsciAPIs(textLexer);
    apis->add(QString("move"));//可自行添加
    apis->add(QString("moive"));
    if(apis->load(QString(":/images/apis")))
        qDebug()<<"读取成功";
    else
        qDebug()<<"读取失败";
    apis->prepare();
    textEdit->setAutoCompletionSource(QsciScintilla::AcsAll);   //设置自动完成所有项
    textEdit->setAutoCompletionCaseSensitivity(true);           //设置自动补全大小写敏感
    textEdit->setAutoCompletionFillupsEnabled(true);
    textEdit->setAutoCompletionThreshold(1);                    //每输入1个字符就出现自动完成的提示


    //2. 行号显示区域 ok
    textEdit->setMarginType(0, QsciScintilla::NumberMargin);
    textEdit->setMarginLineNumbers(0, true);
    textEdit->setMarginWidth(0,30);



    //断点设置区域？？
    //https://qscintilla.com/symbol-margin/
    textEdit->setMarginType(1, QsciScintilla::SymbolMargin);
    textEdit->setMarginLineNumbers(1, false);
    textEdit->setMarginWidth(1,20);
    textEdit->setMarginSensitivity(1, true);    //设置是否可以显示断点
    textEdit->setMarginsBackgroundColor(QColor("#bbfaae"));
    textEdit->setMarginMarkerMask(1, 0x02);
    connect(textEdit, SIGNAL(marginClicked(int, int, Qt::KeyboardModifiers)),this,
            SLOT(on_margin_clicked(int, int, Qt::KeyboardModifiers)));
    textEdit->markerDefine(QsciScintilla::Circle, 1);//断点形状大小
    textEdit->setMarkerBackgroundColor(QColor("#ee1111"), 1);//断点颜色


    //单步执行显示区域 ？？？
    textEdit->setMarginType(2, QsciScintilla::SymbolMargin);
    textEdit->setMarginLineNumbers(2, false);
    textEdit->setMarginWidth(2, 20);
    textEdit->setMarginSensitivity(2, false);
    textEdit->setMarginMarkerMask(2, 0x04);
    textEdit->markerDefine(QsciScintilla::RightArrow, 2);
    textEdit->setMarkerBackgroundColor(QColor("#eaf593"), 2);


    //自动折叠区域 ？？？
    textEdit->setMarginType(3, QsciScintilla::SymbolMargin);
    textEdit->setMarginLineNumbers(3, false);
    textEdit->setMarginWidth(3, 15);
    textEdit->setMarginSensitivity(3, true);


    /*****************以下为zjm添加的测试功能*******************/

    //设置括号匹配 ok
    textEdit->setBraceMatching(QsciScintilla::SloppyBraceMatch);
    //开启自动缩进 ok
    textEdit->setAutoIndent(true);
    //设置缩进的显示方式 ok
    textEdit->setIndentationGuides(QsciScintilla::SC_IV_LOOKBOTH);
    //为选中行添加背景 ok
    textEdit->setCaretLineVisible(true);
    textEdit->setCaretLineBackgroundColor(Qt::lightGray);
    //设置字体 ok
    textEdit->setFont(QFont("Courier New"));
    //设置编码 ok
    textEdit->SendScintilla(QsciScintilla::SCI_SETCODEPAGE,QsciScintilla::SC_CP_UTF8);//设置编码为UTF-8

    /****************以上为zjm添加的测试功能********************/
}
//绑定函数+绑定图片+快捷键+状态栏提示 ok
void MainWindow::createActions()
{
    //新建
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    //打开
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    //保存
    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    //另存为
    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    //退出
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    //剪切
    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

    //复制
    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

    //粘贴
    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    //全选
    selectallAct = new QAction(QIcon(":/images/selectAll.png"),tr("&SelectAll"),this);
    selectallAct->setShortcut(tr("Ctrl+A"));
    selectallAct->setStatusTip(tr("Select all the content in current file"));
    connect(selectallAct, SIGNAL(triggered()), textEdit, SLOT(selectAll()));

    //撤销
    undoAct = new QAction(QIcon(":/images/undo.png"),tr("&Undo"),this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Cancel this operation"));
    connect(undoAct, SIGNAL(triggered()), textEdit, SLOT(undo()));

    //重做
    redoAct = new QAction(QIcon(":/images/redo.png"),tr("&Redo"),this);
    redoAct->setShortcut(tr("Ctrl+Y"));
    redoAct->setStatusTip(tr("Cancel this operation"));
    connect(redoAct, SIGNAL(triggered()), textEdit, SLOT(redo()));


    //替换
    replaceAct = new QAction(QIcon(":/images/find.png"),tr("&Replace"),this);
    replaceAct->setShortcut(tr("Ctrl+H"));
    replaceAct->setStatusTip(tr("Find the specified content in current file and replace"));
    connect(replaceAct, SIGNAL(triggered()), this, SLOT(showReplace()));

    //查找
    findAct = new QAction(QIcon(":/images/find.png"), tr("&Find"), this);
    findAct->setShortcut(tr("Ctrl+F"));
    findAct->setStatusTip(tr("Find the specified content in current file"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFind()));



    //关于
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    //关于Qt
    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    //小细节：初始化复制和粘贴不可点击 需要同copyAvailable的检查结果进行绑定
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));


    /*
     *      ui->textEdit->findFirst(expr,true,false,true,true);
     *      ui->textEdit->findNext();
     *      ui->textEdit->replace(replaceStr);
     */
}
//创建菜单栏 ok
void MainWindow::createMenus()
{
    //文件
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();//添加分隔符
    fileMenu->addAction(exitAct);

    //编辑
    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addAction(selectallAct);
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addAction(replaceAct);
    editMenu->addAction(findAct);

    //menuBar()->addSeparator();

    //TODO 添加菜单栏的运行功能


    //帮助
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}
//创建工具栏 ok
void MainWindow::createToolBars()
{
    //文件
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    //编辑
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addAction(selectallAct);
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);

    //TODO 添加状态栏的运行功能
}
//创建状态栏 ok
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
    //TODO 添加当前行号的提示
}
//读 - 配置文件
void MainWindow::readSettings()
{
    //当我们创建一个Qsettings的对象时，我们需要传递给它两个参数，第一个是你公司或者组织的名称，第二个事你的应用程序的名称
    QSettings settings("Code or die", "C Language Editor");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();//窗口的位置
    QSize size = settings.value("size", QSize(400, 400)).toSize();//窗口的大小
    resize(size);
    move(pos);
}
/*
 * QSetting 配置文件
 * https://www.cnblogs.com/claireyuancy/p/7095249.html
 * https://blog.csdn.net/komtao520/article/details/79636665
 * https://blog.csdn.net/qq1071247042/article/details/52892342
 */
//写 - 配置文件
void MainWindow::writeSettings()
{
    QSettings settings("Code or die", "C Language Editor");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}
//检查该文件是否进行更改 并提示用户该文件未保存，是否需要保存 ok
bool MainWindow::maybeSave()
{
    if (textEdit->isModified()) {//更改过 可能需要保存
        int ret = QMessageBox::warning(this, tr("Application"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape);
        if (ret == QMessageBox::Yes)
            return save();//返回保存结果
        else if (ret == QMessageBox::Cancel)
            return false;//点击取消 不进行关闭、新建等造成当前文件丢失的操作
    }
    return true;//没有进行过更改
}
//读取文件 ok
void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream File_in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setText(File_in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFileName(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}
//保存文件 ok
bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream File_out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    File_out << textEdit->text();
    QApplication::restoreOverrideCursor();

    setCurrentFileName(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}
//设置当前文件名 并在标题中显示 ok
void MainWindow::setCurrentFileName(const QString &fileName)
{
    curFile = fileName;
    textEdit->setModified(false);
    setWindowModified(false);

    QString TitleName;
    if (curFile.isEmpty())
        TitleName = "untitled.txt";
    else
        TitleName = QFileInfo(curFile).fileName();//shownName = strippedName(curFile);

    setWindowTitle(tr("%1[*] - %2").arg(TitleName).arg(tr("TextApplication")));//显示在标题
}


//自定义槽函数
void MainWindow::showReplace()
{
    replaceDialog.show();
}

void MainWindow::showFind()
{
    findDialog.show();
}

void MainWindow::handleFindByTarget(QString target, bool cs, bool forward)
{
    //非正则表达式、大小写敏感、无需完整匹配单词、突出显示？、搜索方向
    if(!textEdit->findFirst(target, false, cs, false, true, forward))
    {
        QMessageBox msg(NULL);

        msg.setWindowTitle("Find");
        msg.setText("Can not find \"" + target + "\"");
        msg.setIcon(QMessageBox::Information);
        msg.setStandardButtons(QMessageBox::Ok);

        msg.setWindowFlags(Qt::WindowStaysOnTopHint);
        msg.exec();
    }
}

void MainWindow::handleReplaceSelect(QString target, QString to, bool cs, bool forward, bool replaceAll)
{
    if(!textEdit->hasSelectedText())
        handleFindByTarget(target, cs, forward);

    if(!replaceAll){
        textEdit->replace(to);
        handleFindByTarget(target, cs, forward);
    }
    else {
        while(textEdit->findFirst(target, false, cs, false, true, forward))
            textEdit->replace(to);

        QMessageBox msg(NULL);

        msg.setWindowTitle("Replace");
        msg.setText("Finished");
        msg.setIcon(QMessageBox::Information);
        msg.setStandardButtons(QMessageBox::Ok);

        msg.setWindowFlags(Qt::WindowStaysOnTopHint);
        msg.exec();
    }
}
