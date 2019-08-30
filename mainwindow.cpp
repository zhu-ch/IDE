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
#include <QDialog>
#include <QFontDialog>
#include <QColorDialog>
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
#include <QKeyEvent>
#include <Qsci/qsciscintilla.h>
#include<set>

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

    QWidget* mainWidget = new QWidget;      //主窗口



    setCentralWidget(textEdit);
    createActions();//点击按钮与函数进行绑定
    createMenus();//菜单栏
    createToolBars();//工具栏
    createStatusBar();//状态栏

    mainLayout = new QVBoxLayout;
    setTextEdit();//代码编辑区
    initLogtext();//编译信息提示区域

    //设置layout布局
    mainLayout->addWidget(textEdit, 0);
    mainLayout->addWidget(LogText);
    mainWidget->setLayout(mainLayout);
    this->setCentralWidget(mainWidget);


    readSettings();

    connect(textEdit, SIGNAL(textChanged()),this, SLOT(documentWasModified()));//点击断点区域 绑定有关断点的函数

    connect(textEdit,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(do_cursorChanged()));//实时显示当前光标所在行函数

    setCurrentFileName("");

    //处理查找、替换窗口的信号
    connect(&findDialog, SIGNAL(findByTarget(QString, bool, bool)), this, SLOT(handleFindByTarget(QString, bool, bool)));
    connect(&replaceDialog, SIGNAL(findByTarget(QString, bool, bool)), this, SLOT(handleFindByTarget(QString, bool, bool)));
    connect(&replaceDialog, SIGNAL(replaceSelect(QString, QString, bool, bool, bool)),
            this, SLOT(handleReplaceSelect(QString, QString, bool, bool, bool)));

    move(0, 0);
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
//ctrl+wheel 字体放大缩小
void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(QApplication::keyboardModifiers () == Qt::ControlModifier){
        if(event->delta()>0)//鼠标往前转
            textEdit->zoomIn();//放大
        else
            textEdit->zoomOut();//缩小
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
//编译
bool MainWindow::LoadLogFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly))
    {
        return false;
    }

    QTextStream in(&file);
//    qDebug() << in.readAll();
    QString logInfo = in.readAll();

    bool isSucess;
    //根据log文件的空与否判断编译是否有误
    if (logInfo.isEmpty())
    {
        logInfo = "--编译成功 在控制台执行程序--";
        isSucess = true;
    }
    else
    {
        logInfo = "--编译失败：错误信息如下--\r\n"+ logInfo;
        isSucess = false;
        //标出错误行数
//        int errorline = GeterrorLine(logInfo);
//        this->SeterrorMarker(errorline);
    }

    //将编译信息填写到编译信息框
    QApplication::setOverrideCursor(Qt::WaitCursor);
    LogText->setText(logInfo);
    QApplication::restoreOverrideCursor();

    return isSucess;
}
void MainWindow::mycompile(){
    //QMessageBox::information(NULL, "Title", "Test", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(maybeSave()){
        /*预编译部分*/
        //QString filename = QFileDialog::getSaveFileName(this);
        QString filename = curFile;
        FILE *p = fopen(filename.toStdString().data(),"r");
        if(p == NULL) return ;

        QString cppfile = filename +".c";
        //qDebug()<<tr("cppfile")<<tr(cppfile.toStdString().data());
        FILE *p1 = fopen(cppfile.toStdString().data(),"w");
        if(p1 == NULL) return ;

        QString str;
        while(!feof(p))
        {
            char buf[1024] = {0};
            fgets(buf,sizeof(buf),p);
            str += buf;
        }
        fputs(str.toStdString().data(),p1);
        fclose(p);
        fclose(p1);
        QString cmd;
        const char *s = filename.toStdString().data();
        cmd.sprintf("gcc -o %s.exe %s.c 2>%s.log",s,s,s);
        system(cmd.toStdString().data());//先编译

//        //如何删除那个临时文件呢
//        cmd = filename.replace("/","\\") + ".c";
//        remove(cmd.toStdString().data());


        //判断是否编译成功
        QString LOG = filename.toStdString().data();
        if(LoadLogFile(LOG+".log")){
            cmd = filename + ".exe";
            system(cmd.toStdString().data());//再运行
        }

    }else{
        QMessageBox::information(NULL, "Title", "文件需保存成功", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
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
void MainWindow::do_cursorChanged(){
    int ColNum;
    int RowNum;
    textEdit->getCursorPosition(&ColNum,&RowNum);

    QString Tip = QString("Current ColNum： ")+QString::number(ColNum)+QString("     Current RowNum： ")+QString::number(RowNum);////////////////////////////////
    setStatusTip(Tip);
    //const QTextCursor qcursor = textEdit->textCursor();
    //int ColNum = qcursor.columnNumber();
    //int RowNum = qcursor.blockCount();
    //int row = textEdit->document()->blockCount();

}

//代码编辑区
void MainWindow::setTextEdit()
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
//初始化编译信息显示区域
void MainWindow::initLogtext()
{
    LogText = new QTextEdit;
    LogText->setReadOnly(true);     //设置日志编辑器不可编辑       //todo 考虑这里控制台？？

    LogText->setFixedHeight(115);
    LogText->setText(tr("--编译信息显示区域--"));
//    mainLayout->addWidget(LogText);
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
    replaceAct = new QAction(QIcon(":/images/replace.png"),tr("&Replace"),this);
    replaceAct->setShortcut(tr("Ctrl+H"));
    replaceAct->setStatusTip(tr("Find the specified content in current file and replace"));
    connect(replaceAct, SIGNAL(triggered()), this, SLOT(showReplace()));

    //查找
    findAct = new QAction(QIcon(":/images/find.png"), tr("&Find"), this);
    findAct->setShortcut(tr("Ctrl+F"));
    findAct->setStatusTip(tr("Find the specified content in current file"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showFind()));

    //change icon
    changeAct = new QAction(QIcon(":/images/change.png"),tr("&Change"),this);
    changeAct->setShortcut(tr("F2"));
    changeAct->setStatusTip(tr("Change the name of the selected variable"));
    connect(changeAct, SIGNAL(triggered()),this,SLOT(change_name()));


    //编译
    compileAct = new QAction(QIcon(":/images/compile.png"),tr("&Compile"),this);
    compileAct->setShortcut(tr("Ctrl+B"));
    compileAct->setStatusTip(tr("Find the specified content in current file"));
    connect(compileAct, SIGNAL(triggered()), textEdit, SLOT(undo()));

    //运行
    runAct = new QAction(QIcon(":/images/run.png"),tr("&Run"),this);
    runAct->setShortcut(tr("Ctrl+R"));
    runAct->setStatusTip(tr("Find the specified content in current file"));
    connect(runAct, SIGNAL(triggered()), this, SLOT(mycompile()));

    //字体样式和大小
    fontAct = new QAction(QIcon(":/images/font.png"),tr("&Font"),this);
    fontAct->setShortcut(tr("Ctrl+Shift+F"));
    fontAct->setStatusTip(tr("change the style and size of the font in current file"));
    connect(fontAct, SIGNAL(triggered()), this, SLOT(showFont()));

    //字体颜色
    colorAct = new QAction(QIcon(":/images/color.png"),tr("&Color"),this);
    colorAct->setShortcut(tr("Ctrl+Shift+C"));
    colorAct->setStatusTip(tr("change the color of the font in current file"));
    connect(colorAct, SIGNAL(triggered()), this, SLOT(showColor()));

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
    editMenu->addAction(changeAct);


    //编译运行
    compileMenu = menuBar()->addMenu(tr("&Compile - Run"));
    compileMenu->addAction(compileAct);
    compileMenu->addAction(runAct);


    //帮助
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    //格式
    formMenu = menuBar()->addMenu(tr("&Form"));
    formMenu->addAction(fontAct);
    formMenu->addAction(colorAct);
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
    editToolBar->addAction(findAct);
    editToolBar->addAction(replaceAct);
    editToolBar->addAction(changeAct);

    //编译运行
    compileToolBar = addToolBar(tr("Compile"));
    compileToolBar->addAction(compileAct);
    compileToolBar->addAction(runAct);

    //格式
    formToolBar = addToolBar(tr("Form"));
    formToolBar->addAction(fontAct);
    formToolBar->addAction(colorAct);
}
//创建状态栏 ok
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
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

/*
 * author zch
 * description 查找替换功能槽函数
 * date 2019/8/29
 * */
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
    //非正则表达式、【是否】大小写敏感、无需完整匹配单词、选中、搜索方向
    if(!textEdit->findFirst(target, false, cs, false, true, forward))
    {
        QMessageBox msg(NULL);

        msg.setWindowTitle("Find");
        msg.setText("Can not find \"" + target + "\"");
        msg.setIcon(QMessageBox::Information);
        msg.setStandardButtons(QMessageBox::Ok);

        msg.setWindowFlags(Qt::WindowStaysOnTopHint);//置顶
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

/*
 * author zjm
 * description 字体槽函数
 * date 2019/8/30
 * */
void MainWindow::showFont()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok);
    if (ok)
    {
        qDebug()<<f.toString().data();
        textEdit->setFont(f);
        //fontLineEdit->setFont(f);
    }
}

void MainWindow::showColor()
{
    QColor c = QColorDialog::getColor(Qt::blue);
    if (c.isValid())
    {
        //colorFrame->setPalette(QPalette(c));
        textEdit->setColor(c);
    }
}


/*
 * author zll
 * description 变量名替换槽函数
 * date 2019/8/30
 * */
void MainWindow::change_name(){

    int line = 0, index = 0;
    textEdit->getCursorPosition(&line,&index);//get cursor postion


    variableName = textEdit->wordAtLineIndex(line,index);//get the word around the cursor
    //qDebug() << variableName<<variableName.size();

    if(variableName.size()<=0){//todo   how to know this name isn't keyword and string??
        QMessageBox *box = new QMessageBox("Notice",
                    "Invalid variableName.",
                    QMessageBox::NoIcon,
                    QMessageBox::Ok | QMessageBox::Default,
                    QMessageBox::Cancel | QMessageBox::Escape,
                    0
                    );
      box->setModal(true);
      box->show();

      box->exec();
    }
    else{
        qDebug()<<"this";
        //QLineEdit *lineEdit = new QLineEdit();
        lineEdit = new QLineEdit();
        lineEdit->setParent(textEdit);
        lineEdit->resize(150,30);
        lineEdit->move(textEdit->width()-150,0);
        lineEdit->show();
        connect(lineEdit,SIGNAL(returnPressed()),this,SLOT(chang_all_name()));
    }
}

void MainWindow::chang_all_name(){
        std::set<char> chr;
        if(chr.empty()) chr.clear();
        for(int i=0;i<26;i++){
            chr.insert(65+i);
            chr.insert(97+i);
        }
        for(int i=0;i<10;i++){
            chr.insert('0'+i);
        }
        chr.insert('_');

        QString rplc = lineEdit->text();

        while(textEdit->findFirst(variableName,0,1,1,1)){
            textEdit->replace(rplc) ;
        }
//        QMessageBox *box = new QMessageBox("Notice",
//                    "Done.",
//                    QMessageBox::NoIcon,
//                    QMessageBox::Ok | QMessageBox::Default,
//                    QMessageBox::Cancel | QMessageBox::Escape,
//                    0
//                    );
        QMessageBox box;
        box.setWindowTitle(tr("Information"));
        box.setIcon(QMessageBox::Information);
        box.setText(tr("Rename success!"));
        box.setStandardButtons(QMessageBox::Yes);
          //box->setModal(true);

          delete lineEdit;
          lineEdit = nullptr;
          box.show();

          box.exec();

}

