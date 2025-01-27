#include "mainwindow.h"
#include "replacedialog.h"
#include "finddialog.h"
#include "runthread.h"
#include "debugdialog.h"

/*
 * author zjm
 * description 词法分析器
 * date 2019/8/29
 * */
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

MainWindow::MainWindow()
{
    textEdit = new QsciScintilla;
    keyPressEater = new MyKeyPressEater;

    QWidget* mainWidget = new QWidget;      //主窗口
    QWidget* rightAreaWidget = new QWidget;

    setCentralWidget(textEdit);
    createActions();//点击按钮与函数进行绑定
    createMenus();//菜单栏
    createToolBars();//工具栏
    init_statusBar();//状态栏
    bindSignals();//绑定信号
    mainLayoutV = new QVBoxLayout;
    mainLayoutH = new QHBoxLayout;
    setTextEdit();//代码编辑区
    initLogtext();//编译信息提示区域

    //树形控件
    //QSplitter *splitter = new QSplitter;
    //QDirModel *model = new QDirModel;
    //myqtreeview = new MyQTreeView(splitter);
    myqtreeview = new MyQTreeView();
    myqtreeview->model = new QDirModel;
    myqtreeview->setModel(myqtreeview->model);
    fileDir = "D:";
    myqtreeview->setRootIndex(myqtreeview->model->index(fileDir));
    myqtreeview->setColumnWidth(0, 280);
    myqtreeview->hideColumn(1);
    myqtreeview->hideColumn(2);
    myqtreeview->setMaximumWidth(400);
    connect(myqtreeview, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(myTreeViewOpenFile(QModelIndex)));
    //设置layout布局
    mainLayoutV->addWidget(textEdit, 0);
    mainLayoutV->addWidget(LogText);
    //mainLayoutV->addWidget(myqtreeview);//添加树形控件
    rightAreaWidget->setLayout(mainLayoutV);
    mainLayoutH->addWidget(myqtreeview);
    mainLayoutH->addWidget(rightAreaWidget);
    mainWidget->setLayout(mainLayoutH);
    this->setCentralWidget(mainWidget);


    readSettings();
    setCurrentFileName("");
    move(0, 0);

    indicNum = textEdit->indicatorDefine(QsciScintilla::TextColorIndicator);//function highlighter
    textEdit->setIndicatorForegroundColor(Qt::darkYellow, indicNum);//function color
    textEdit->setIndicatorHoverStyle(QsciScintilla::ThinCompositionIndicator, indicNum);
    textEdit->setIndicatorHoverForegroundColor(Qt::blue, indicNum);
    iconCPP = textLexer->keywords(1);
    iconCPP += textLexer->keywords(3);

    isAnnotationHide = false;
}

/*
 * author zjm
 * description 关闭窗口事件
 * date 2019/8/26
 * */
void MainWindow::closeEvent(QCloseEvent *event){
    if (maybeSave()) {//当关闭时 提示是否保存后 用户选择保存（并成功）或不保存
        writeSettings();
        event->accept();
    }
    else //当关闭时 提示是否保存后 用户选择取消
        event->ignore();
}

/*
 * author zjm
 * description ctrl+wheel 字体放大缩小
 * date 2019/8/26
 * */
void MainWindow::wheelEvent(QWheelEvent *event){
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        if (event->delta() > 0)//鼠标往前转
            textEdit->zoomIn();//放大
        else
            textEdit->zoomOut();//缩小
    }
}


/*
 * author zch
 * description 绑定信号
 * date 2019/8/29
 * */
void MainWindow::bindSignals(){
    connect(textEdit, SIGNAL(textChanged()),this, SLOT(funcHighlighter()));
    connect(textEdit, SIGNAL(textChanged()),this, SLOT(documentWasModified()));//点击断点区域 绑定有关断点的函数
    connect(textEdit,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(do_cursorChanged()));//实时显示当前光标所在行函数

    //处理查找、替换窗口的信号
    connect(&findDialog, SIGNAL(findByTarget(QString, bool, bool)), this, SLOT(handleFindByTarget(QString, bool, bool)));
    connect(&replaceDialog, SIGNAL(findByTarget(QString, bool, bool)), this, SLOT(handleFindByTarget(QString, bool, bool)));
    connect(&replaceDialog, SIGNAL(replaceSelect(QString, QString, bool, bool, bool)),
            this, SLOT(handleReplaceSelect(QString, QString, bool, bool, bool)));

    //键盘监听
    connect(keyPressEater, SIGNAL(keyPressSiganl_puncComplete(int)), this, SLOT(handlePuncComplete(int)));
    connect(textEdit,SIGNAL(indicatorReleased (int, int, Qt::KeyboardModifiers)),this,SLOT(jumpDefination(int,int,Qt::KeyboardModifiers)));

    //更新单步执行
    connect(&debugDialog, SIGNAL(signalUpdateMarker(int)), this, SLOT(updateLineNumberSlot(int)));
    connect(&debugDialog, SIGNAL(signalClearMarker()), this, SLOT(clearMarker()));

    //新需求
    connect(textEdit,SIGNAL(textChanged()),this,SLOT(recordPos()));
}


/*
 * author zjm
 * description 文件功能
 * date 2019/8/29
 * */
//新建
void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName("");
    }
}
//打开
void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        qDebug()<<fileName.toStdString().data();
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}
//打开文件夹
void MainWindow::openfolder()
{
    if (maybeSave()) {
        fileDir = QFileDialog::getExistingDirectory(this);
        qDebug()<<fileDir;
        myqtreeview->reset();
        myqtreeview->model = new QDirModel;
        myqtreeview->setModel(myqtreeview->model);
        myqtreeview->setRootIndex(myqtreeview->model->index(fileDir));
    }
}
//保存
bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}
//另存为
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
    QString logInfo = in.readAll();

    bool isSucess;
    //根据log文件的空与否判断编译是否有误
    if (logInfo.isEmpty())
    {
        qDebug()<<"ok";
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

/*
 * author zjm
 * description 单文件编译
 * date 2019/8/30
 *
 * mender zch
 * details
 *      1.void -> bool
 *      2.add "-g" in command
 * date 2019/9/8
 * */
bool MainWindow::mycompile(){
    if(maybeSave()){
        /*预编译部分*/
        //QString filename = QFileDialog::getSaveFileName(this);
        QString filename = curFile;
        FILE *p = fopen(filename.toStdString().data(),"r");
        if(p == NULL) return false;

        QString cppfile = filename +".c";
        //qDebug()<<tr("cppfile")<<tr(cppfile.toStdString().data());
        FILE *p1 = fopen(cppfile.toStdString().data(),"w");
        if(p1 == NULL) return false;

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
        cmd.sprintf("gcc -g -o %s.exe %s.c 2>%s.log",s,s,s);//-g是必须的
        system(cmd.toStdString().data());//先编译


        //判断是否编译成功
        QString LOG = filename.toStdString().data();
        if(LoadLogFile(LOG+".log")){
            qDebug()<<"可以运行";
            return true;
        }
        else{
            QMessageBox::information(NULL, "Compile", "编译失败", QMessageBox::Yes);
            return false;
        }
    }
    else{
        QMessageBox::information(NULL, "Compile", "文件需保存成功", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return false;
    }
}

void MainWindow::myrun(){
    QString LOG = curFile.toStdString().data();
    if(LoadLogFile(LOG+".log")){
        qDebug()<<"可以运行";
        QString cmd = curFile + ".exe && pause";
        RunThread *rthread = new RunThread(cmd);
        connect(rthread, &RunThread::finished, rthread, &QObject::deleteLater);
        rthread->start();
    }else{
        qDebug()<<"编译失败";
    }
}

void MainWindow::compile_run(){
    qDebug()<<"compile_run";
    if(maybeSave()){
        /*预编译部分*/
        //QString filename = QFileDialog::getSaveFileName(this);
        QString filename = curFile;
        FILE *p = fopen(filename.toStdString().data(),"r");
        if(p == NULL) return ;

        QString cppfile = filename +".c";
        qDebug()<<tr("cppfile")<<tr(cppfile.toStdString().data());
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

        //判断是否编译成功
        QString LOG = filename.toStdString().data();
        if(LoadLogFile(LOG+".log")){
            QString cmd = curFile + ".exe && pause";
            RunThread *rthread = new RunThread(cmd);
            connect(rthread, &RunThread::finished, rthread, &QObject::deleteLater);
            rthread->start();
        }
        else
            QMessageBox::information(NULL, "Compile", "编译失败", QMessageBox::Yes);

    }else{
        QMessageBox::information(NULL, "Compile", "文件需保存成功", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
}

QList<QString> MainWindow::findHead(){//寻找多文件编译的外部头文件
    qDebug()<<"进入查找";
    int block_number_1;int block_number_2;int column_number_1;int column_number_2;
    int include_num;int temp_block_number;int temp_column_number;
    QList<QString>include_list;//存储所有的头文件.h前的内容
    include_num=0;//总共找到的带""的头文件数量
    QString star="#include \"";
    QString end=".h\"";
    temp_block_number=0;temp_column_number=0;//存储找到的第一个的位置，这样查够一轮后退出
    if(textEdit->findFirst(star, false, true, false, true, true)){//此时光标在#include"后
        include_num++;
        textEdit->getCursorPosition(&block_number_1,&column_number_1);
        temp_block_number=block_number_1;temp_column_number=column_number_1;
        textEdit->findFirst(end, false, true, false, true, true);//此时光标在.h"后
        textEdit->getCursorPosition(&block_number_2,&column_number_2);
        textEdit->setSelection(block_number_1,column_number_1,block_number_2,column_number_2-3);//-3是为了去除.h"三个字符
        include_list.append(textEdit->selectedText());
        while(textEdit->findFirst(star, false, true, false, true, true)){
            include_num++;
            textEdit->getCursorPosition(&block_number_1,&column_number_1);
            if(temp_block_number==block_number_1&&temp_column_number==column_number_1){//搜够一轮又回来了，结束
                include_num--;
                break;
            }
            textEdit->findFirst(end, false, true, false, true, true);
            textEdit->getCursorPosition(&block_number_2,&column_number_2);
            textEdit->setSelection(block_number_1,column_number_1,block_number_2,column_number_2-3);
            qDebug()<<textEdit->selectedText();
            include_list.append(textEdit->selectedText());
        }
    }

    return include_list;
}

void MainWindow::all_compile(){
    if(maybeSave()){
        /*预编译部分*/
        //QString filename = QFileDialog::getSaveFileName(this);
        QString filename = curFile;
        FILE *p = fopen(filename.toStdString().data(),"r");
        if(p == NULL) return ;

        QString cppfile = filename +".cpp";
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

        //cmd = "g++ -o "+filename+".exe -g";
        cmd = "g++ -o "+filename+".exe ";
        cmd += filename+".cpp";

        //获取路径
        QString path="";
        int index_path_end;
        for(int i=filename.size();i>=0;i--){
            //qDebug()<<filename.at(i);
            if(filename.at(i)=="/"){
                index_path_end=i;
                break;
            }
        }
        //qDebug()<<index_path_end;
        for(int i=0;i<=index_path_end;i++){
            path+=filename.at(i);
        }
        //qDebug()<<path;

        //进行拼接
        QList<QString> headList = findHead();
        qDebug()<<headList.size();
        QString head;
        for(int i=0;i<headList.size();i++){
            head = headList.at(i);
            qDebug()<<head;
            cmd += " "+path+head+".cpp";
        }

        //cmd += " 2>"+filename+".log -g";
        cmd += " 2>"+filename+".log";
        qDebug()<<cmd.toStdString().data();
        system(cmd.toStdString().data());

        //判断是否编译成功
        QString LOG = filename.toStdString().data();
        if(LoadLogFile(LOG+".log")){
            qDebug()<<"可以运行";
        }
        else
            QMessageBox::information(NULL, "Compile", "编译失败", QMessageBox::Yes);

    }else{
        QMessageBox::information(NULL, "Compile", "文件需保存成功", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
}

void MainWindow::all_run(){
    QString LOG = curFile.toStdString().data();
    if(LoadLogFile(LOG+".log")){
        qDebug()<<"可以运行";
        QString cmd = curFile + ".exe && pause";
        RunThread *rthread = new RunThread(cmd);
        connect(rthread, &RunThread::finished, rthread, &QObject::deleteLater);
        rthread->start();
    }else{
        qDebug()<<"编译失败";
    }
}

//关于
void MainWindow::about(){
   QMessageBox::about(this, tr("About us"),
            tr("MiniIDE C语言编辑器 开发团队"
               "1120173305 张佳明\n"
               "1120171227 朱长昊\n"
               "1120172167 湛蓝蓝\n"
               "1120172164 刘震宇\n"
               "1120171491 蒋雨彤\n"));
}

void MainWindow::documentWasModified(){
    setWindowModified(textEdit->isModified());
}


/*
 * author zjm zch
 * description 断点
 * date 2019/8/29
 * modify 2019/9/7
 * */
void MainWindow::on_margin_clicked(int margin, int line, Qt::KeyboardModifiers state)
{
    Q_UNUSED(state);

    if (margin == 1) {
        if (textEdit->markersAtLine(line) != 0) {//当前位置有断点 去掉
            textEdit->markerDelete(line, 1);
            for(std::vector<int>::iterator it = breakpoints.begin(); it != breakpoints.end(); ){
                if(*it == line + 1)
                    it = breakpoints.erase(it);
                else
                   ++it;
            }
            for(std::vector<int>::iterator it = breakpoints.begin(); it != breakpoints.end(); ){
                qDebug()<<*it;
                it++;
            }
            qDebug()<<"end";
            debugDialog.setBreakpoints(breakpoints);
        }
        else {//当前位置无断点 添加
            textEdit->markerAdd(line, 1);
            breakpoints.push_back(line + 1);//从1开始
            for(std::vector<int>::iterator it = breakpoints.begin(); it != breakpoints.end(); ){
                qDebug()<<*it;
                it++;
            }
            qDebug()<<"end";
            debugDialog.setBreakpoints(breakpoints);
        }
    }
}

/*
 * author lzy zch
 * description 行号列号
 * date 2019/9/4
 * modify 2019/9/8
 * */
void MainWindow::do_cursorChanged(){
    textEdit->getCursorPosition(&cursorLine,&cursorIndex);

    QString Tip = QString("Current ColNum： ") + QString::number(cursorIndex + 1)
            + QString("     Current RowNum： ") + QString::number(cursorLine + 1);
    first_statusLabel->setText(Tip);
}


/*
 * author zjm zch
 * description 代码编辑区
 * date 2019/8/29
 * modify 2019/8/31
 * */
void MainWindow::setTextEdit()
{
    textLexer = new QscilexerCppAttach;                                             //绑定Cpp的关键字
    textLexer->setColor(QColor(Qt:: green),QsciLexerCPP::CommentLine);              //设置自带的注释行为绿色
    textLexer->setColor(QColor(Qt:: yellow),QsciLexerCPP::KeywordSet2);             //设置自定义关键字的颜色为黄色
    textEdit->setLexer(textLexer);
    textEdit->installEventFilter(keyPressEater);

    //1. 设置自动补全的字符串和补全方式
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


    //2.左边功能栏
    //2.1.行号显示区域
    textEdit->setMarginType(0, QsciScintilla::NumberMargin);
    textEdit->setMarginLineNumbers(0, true);
    textEdit->setMarginWidth(0,30);

    //2.2断点设置区域？？
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

    //2.3单步执行显示区域 ？？
    textEdit->setMarginType(2, QsciScintilla::SymbolMargin);
    textEdit->setMarginLineNumbers(2, false);
    textEdit->setMarginWidth(2, 20);
    textEdit->setMarginSensitivity(2, false);
    textEdit->setMarginMarkerMask(2, 0x04);
    textEdit->markerDefine(QsciScintilla::RightArrow, 2);
    textEdit->setMarkerBackgroundColor(QColor("#eaf593"), 2);

    //2.4代码折叠
    textEdit->setMarginType(3, QsciScintilla::SymbolMargin);
    textEdit->setMarginLineNumbers(3, false);
    textEdit->setMarginWidth(3, 15);
    textEdit->setMarginSensitivity(3, true);
    textEdit->setFolding(QsciScintilla::BoxedTreeFoldStyle, 3);

    //3.设置括号匹配
    textEdit->setBraceMatching(QsciScintilla::SloppyBraceMatch);

    //4.缩进
    //开启自动缩进
    textEdit->setAutoIndent(true);
    //设置缩进的显示方式
    textEdit->setIndentationGuides(QsciScintilla::SC_IV_LOOKBOTH);

    //5.为选中行添加背景
    textEdit->setCaretLineVisible(true);
    textEdit->setCaretLineBackgroundColor(Qt::lightGray);

    //6.设置字体
    textEdit->setFont(QFont("Courier New"));

    //7.设置编码为UTF-8
    textEdit->SendScintilla(QsciScintilla::SCI_SETCODEPAGE,QsciScintilla::SC_CP_UTF8);
}


/*
 * author zjm
 * description 初始化编译信息显示区域
 * date 2019/8/29
 * */
void MainWindow::initLogtext()
{
    LogText = new QTextEdit;
    LogText->setReadOnly(true);     //设置日志编辑器不可编辑       //todo 考虑这里控制台？？

    LogText->setFixedHeight(115);
    LogText->setText(tr("--编译信息显示区域--"));
}


/*
 * author zjm zch zll
 * description 槽函数绑定
 * date 2019/8/29
 * modify 2019/8/30
 * */
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

    //打开文件夹
    openfolderAct = new QAction(QIcon(":/images/openfolder.png"), tr("&OpenFolder..."), this);
    openfolderAct->setShortcut(tr("Ctrl+Shift+O"));
    openfolderAct->setStatusTip(tr("Open an existing file folder"));
    connect(openfolderAct, SIGNAL(triggered()), this, SLOT(openfolder()));

    //保存
    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    //另存为
    saveAsAct = new QAction(QIcon(":/images/save-as.png"), tr("&Save As"), this);
    saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
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
    undoAct->setShortcut(tr("ctrl+Z"));
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

    //变量重命名
    changeAct = new QAction(QIcon(":/images/change.png"),tr("&Change"),this);
    changeAct->setShortcut(tr("F2"));
    changeAct->setStatusTip(tr("Change the name of the selected variable"));
    connect(changeAct, SIGNAL(triggered()),this,SLOT(change_name()));


    //设置注释
//    annotation = new QAction(QIcon(":/images/annotation.png"),tr("&annotation"),this);
//    QShortcut* shortcut = new QShortcut(QKeySequence("Shift+Ctrl+/"),this);
//    connect(shortcut,SIGNAL(activated()),this,SLOT(Annotation()));
    annotation = new QAction(QIcon(":/images/annotation.png"),tr("&annotation"),this);
    annotation->setShortcut(tr("Ctrl+Shift+/"));
    annotation->setStatusTip(tr("annotation"));
    connect(annotation, SIGNAL(triggered()),this,SLOT(Annotation()));


    //整体代码格式化
    allFormattingAct = new QAction(QIcon(":/images/format.png"),tr("&Formatting"),this);
    allFormattingAct->setShortcut(tr("F3"));
    allFormattingAct->setStatusTip(tr("Formatting"));
    connect(allFormattingAct, SIGNAL(triggered()), this, SLOT(Formatting_All()));

    //新需求
    annotateHideAct = new QAction(QIcon(":/images/annotate.png"),tr("&HideOrShowAnnotate"),this);
    annotateHideAct->setShortcut(tr("F8"));
    annotateHideAct->setStatusTip(tr("Hide or show all annotations"));
    connect(annotateHideAct,SIGNAL(triggered()),this,SLOT(annotate_hide_and_show()));


    //编译
    compileAct = new QAction(QIcon(":/images/compile.png"),tr("&Compile"),this);
    compileAct->setShortcut(tr("Ctrl+B"));
    compileAct->setStatusTip(tr("Find the specified content in current file"));
    connect(compileAct, SIGNAL(triggered()), this, SLOT(mycompile()));

    //运行
    runAct = new QAction(QIcon(":/images/run.png"),tr("&Run"),this);
    runAct->setShortcut(tr("Ctrl+R"));
    runAct->setStatusTip(tr("Find the specified content in current file"));
    connect(runAct, SIGNAL(triggered()), this, SLOT(myrun()));

    //编译运行
    CompileRunAct = new QAction(QIcon(":/images/compile_run.png"),tr("&Compile_Run"),this);
    CompileRunAct->setShortcut(tr("Alt+R"));
    CompileRunAct->setStatusTip(tr("Find the specified content in current file"));
    connect(CompileRunAct, SIGNAL(triggered()), this, SLOT(compile_run()));

    //多文件编译
    allCompileAct = new QAction(QIcon(":/images/all-compile.png"),tr("&All_Compile"),this);
    allCompileAct->setShortcut(tr("Ctrl+Shift+B"));
    allCompileAct->setStatusTip(tr("Find the specified content in current file"));
    connect(allCompileAct, SIGNAL(triggered()), this, SLOT(all_compile()));

    //多文件运行
    allRunAct = new QAction(QIcon(":/images/all-run.png"),tr("&All_Run"),this);
    allRunAct->setShortcut(tr("Ctrl+Shift+R"));
    allRunAct->setStatusTip(tr("Find the specified content in current file"));
    connect(allRunAct, SIGNAL(triggered()), this, SLOT(all_run()));

    //debug
    debugAct = new QAction(QIcon(":/images/debug.png"),tr("&debug"),this);
    debugAct->setShortcut(tr("F5"));
    debugAct->setStatusTip(tr("Open debug dialog and start debug mode"));
    connect(debugAct, SIGNAL(triggered()), this, SLOT(debugSlot()));

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
    aboutAct = new QAction(tr("&About &us"), this);
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

/*
 * author zjm zch zll
 * description 创建菜单栏
 * date 2019/8/29
 * modify 2019/9/7
 * */
void MainWindow::createMenus()
{
    //文件
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(openfolderAct);
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
    editMenu->addAction(annotation);
    editMenu->addAction(allFormattingAct);
    editMenu->addAction(annotateHideAct);


    //编译运行
    compileMenu = menuBar()->addMenu(tr("&Run"));
    compileMenu->addAction(compileAct);
    compileMenu->addAction(runAct);
    compileMenu->addAction(CompileRunAct);
    compileMenu->addAction(allCompileAct);
    compileMenu->addAction(allRunAct);
    compileMenu->addAction(debugAct);


    //帮助
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    //格式
    formMenu = menuBar()->addMenu(tr("&Form"));
    formMenu->addAction(fontAct);
    formMenu->addAction(colorAct);
}


/*
 * author zjm zch
 * description 创建工具栏
 * date 2019/8/29
 * modify 2019/8/30
 * */
void MainWindow::createToolBars()
{
    //文件
   fileToolBar = addToolBar(tr("File"));
   fileToolBar->addAction(newAct);
   fileToolBar->addAction(openAct);
   fileToolBar->addAction(openfolderAct);
   fileToolBar->addAction(saveAct);
   fileToolBar->addAction(saveAsAct);
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
    editToolBar->addAction(annotation);
    editToolBar->addAction(allFormattingAct);

    //编译运行
    compileToolBar = addToolBar(tr("Compile"));
    compileToolBar->addAction(compileAct);
    compileToolBar->addAction(runAct);
    compileToolBar->addAction(CompileRunAct);
    compileToolBar->addAction(allCompileAct);
    compileToolBar->addAction(allRunAct);
    compileToolBar->addAction(debugAct);

    //格式
    formToolBar = addToolBar(tr("Form"));
    formToolBar->addAction(fontAct);
    formToolBar->addAction(colorAct);
}

/*
 * author zjm
 * description 读取配置文件
 * date 2019/8/29
 * */
/*
 * QSetting 配置文件
 * https://www.cnblogs.com/claireyuancy/p/7095249.html
 * https://blog.csdn.net/komtao520/article/details/79636665
 * https://blog.csdn.net/qq1071247042/article/details/52892342
 */
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
 * author zjm
 * description 写配置文件
 * date 2019/8/29
 * */
void MainWindow::writeSettings()
{
    QSettings settings("Code or die", "C Language Editor");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}


/*
 * author zjm
 * description 退出时检查该文件是否进行更改 并提示用户该文件未保存，是否需要保存
 * date 2019/8/29
 * */
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



/*
 * author zjm
 * description 读取文件
 * date 2019/8/29
 * */
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



/*
 * author zjm
 * description 保存文件
 * date 2019/8/29
 * */
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



/*
 * author zjm
 * description 设置当前文件名 并在标题中显示
 * date 2019/8/29
 * modify 2019/8/30
 * */
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
void MainWindow::showReplace(){
    replaceDialog.show();
}

void MainWindow::showFind(){
    findDialog.show();
}

void MainWindow::handleFindByTarget(QString target, bool cs, bool forward){
    //非正则表达式、【是否】大小写敏感、无需完整匹配单词、选中、搜索方向
    if(!textEdit->findFirst(target, false, cs, false, true, forward))    {
        QMessageBox msg(NULL);

        msg.setWindowTitle("Find");
        msg.setText("Can not find \"" + target + "\"");
        msg.setIcon(QMessageBox::Information);
        msg.setStandardButtons(QMessageBox::Ok);

        msg.setWindowFlags(Qt::WindowStaysOnTopHint);//置顶
        msg.exec();
    }
}

void MainWindow::handleReplaceSelect(QString target, QString to, bool cs, bool forward, bool replaceAll){
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

//void MainWindow::chang_all_name(){
//        std::set<char> chr;
//        if(chr.empty()) chr.clear();
//        for(int i=0;i<26;i++){
//            chr.insert(65+i);
//            chr.insert(97+i);
//        }
//        for(int i=0;i<10;i++){
//            chr.insert('0'+i);
//        }
//        chr.insert('_');

//        QString rplc = lineEdit->text();

//        while(textEdit->findFirst(variableName,0,1,1,1)){
//            textEdit->replace(rplc) ;
//        }
////        QMessageBox *box = new QMessageBox("Notice",
////                    "Done.",
////                    QMessageBox::NoIcon,
////                    QMessageBox::Ok | QMessageBox::Default,
////                    QMessageBox::Cancel | QMessageBox::Escape,
////                    0
////                    );
//        QMessageBox box;
//        box.setWindowTitle(tr("Information"));
//        box.setIcon(QMessageBox::Information);
//        box.setText(tr("Rename success!"));
//        box.setStandardButtons(QMessageBox::Yes);
//          //box->setModal(true);
//        lineEdit->close();
//          delete lineEdit;
//          lineEdit = nullptr;
//          box.show();
//          box.exec();
//}

void MainWindow::chang_all_name(){
    int line = 0, index = 0,now = 0;
   textEdit->getCursorPosition(&line,&index);
   now =textEdit->positionFromLineIndex(line,index);

   QString rplc = lineEdit->text();
   QRegExp re("(\\w+)\\s+[\\,\\w]*" + variableName + "[\\s\\,\\;\\)]+");
   QString text = textEdit->text();
   int pos1= text.indexOf(re);
   int pos2=pos1;
   while(pos1 != -1 &&  iconCPP.count(" " + re.cap(1) + " ") == 0){
       pos1= text.indexOf(re,pos1 + re.matchedLength());//find first defination of this name
   }
     qDebug()<<re.capturedTexts();
   if(pos1 != -1){//has defination
           pos2 = text.indexOf(re,pos1 + re.matchedLength());//find the threshold-- next defination
           while(pos2!=-1){
               qDebug()<<re.capturedTexts();
               if(iconCPP.count(" " + re.cap(1) + " ") != 0){//has found the next defination
                   if(pos2 < now)//still not the threshold
                       pos1 = pos2;
                   else break;
               }
               if(pos2 != -1) pos2 = text.indexOf(re,pos2 + re.matchedLength());
           }

       textEdit->lineIndexFromPosition(pos1,&line,&index);

       if(pos2 == -1) {
               textEdit->findFirst(variableName,0,1,1,0,true,line,index);
               textEdit->replace(rplc) ;
               while(textEdit->findNext()){
                   textEdit->replace(rplc);
               }
       }
       else{
           int lt,it;
           textEdit->lineIndexFromPosition(pos2,&lt,&it);
           textEdit->setSelection(line,index,lt,it);
           textEdit->findFirstInSelection(variableName,0,1,1,0);
           textEdit->replace(rplc);
           while(textEdit->findNext()){
               textEdit->replace(rplc);
           }
        }
   }
   else{
       while(textEdit->findFirst(variableName,0,1,1,0,true)){
           textEdit->replace(rplc) ;
       }
   }

   QMessageBox *box = new QMessageBox("Notice",
               "Done.",
               QMessageBox::NoIcon,
               QMessageBox::Ok | QMessageBox::Default,
               QMessageBox::Cancel | QMessageBox::Escape,
               0
               );
     box->setModal(true);

     lineEdit->close();
     delete lineEdit;
     lineEdit = nullptr;
     box->show();

     box->exec();
     textEdit->setCursorPosition(0,0);
}

/*
 * author zch
 * description 监听键盘输入
 * date 2019/8/31
 * */
bool MyKeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == 40 || keyEvent->key() == 91 || keyEvent->key() == 123
                || keyEvent->key() == 34 || keyEvent->key() == 39||keyEvent->key() ==59
                ||keyEvent->key() ==Qt::Key_Enter||keyEvent->key() ==Qt::Key_Return){
            //qDebug("Ate key press %d", keyEvent->key());
            emit keyPressSiganl_puncComplete(keyEvent->key());
            return true;
        }
        else return QObject::eventFilter(obj, event);
    } else {
        return QObject::eventFilter(obj, event);
    }
}


/*
 * author zch
 * description 标点补全
 * date 2019/8/31
 * */
void MainWindow::handlePuncComplete(int key){
    //qDebug("handleBraceComplete %d", key);
    this->textEdit->getCursorPosition(&cursorLine, &cursorIndex);
    switch (key) {
    case 34://"
        this->textEdit->insert(tr("\"\""));
        break;
    case 39://'
        this->textEdit->insert(tr("''"));
        break;
    case 40://(
        this->textEdit->insert(tr("()"));
        break;
    case 91://[
        this->textEdit->insert(tr("[]"));
        break;
    case 123://{
        this->textEdit->insert(tr("{}"));
        //Formatting_All();
        break;
    case Qt::Key_Enter://两个键都是监听回车
    case Qt::Key_Return:
        this->textEdit->insert(tr("\n"));
        Enter_Formatting(cursorLine);
        return;
     case 59://;
        this->textEdit->insert(tr(";"));
        lineFormatting(cursorLine);
        return;
    default:
        break;
    }
    this->textEdit->setCursorPosition(cursorLine, cursorIndex+1);
}

/*
 * author lzy
 * description 状态栏修改
 * date 2019/9/1
 * */
void MainWindow::init_statusBar()
{
    first_statusLabel = new QLabel; //新建标签
    first_statusLabel->setMinimumSize(400,30); //设置标签最小尺寸
    first_statusLabel->setFrameShape(QFrame::WinPanel); //设置标签形状
    first_statusLabel->setContentsMargins(15,0,0,0);
    //first_statusLabel->setFrameShadow(QFrame::Sunken); //设置标签阴影
    second_statusLabel = new QLabel;
    second_statusLabel->setMinimumSize(400,30);
    second_statusLabel->setFrameShape(QFrame::WinPanel);
    second_statusLabel->setContentsMargins(70,0,0,0);
    //second_statusLabel->setFrameShadow(QFrame::Sunken);
    statusBar()->setStyleSheet("QFrame{border: 0px;}");
    statusBar()->addWidget(first_statusLabel);
    statusBar()->addWidget(second_statusLabel);
    first_statusLabel->setText(tr( "Current ColNum： 0     Current RowNum： 0")); //初始化内容
    second_statusLabel->setText(tr( "Ready"));
}
/*
 * author zjm lzy
 * description 多行注释
 * date 2019/9/2
 * */
void MainWindow::Annotation(){
    textEdit->getSelection(&lineFrom,&indexFrom,&lineTo,&indexTo);
    if(lineFrom>lineTo){
        int temp;
        temp=lineFrom;
        lineFrom=lineTo;
        lineTo=temp;
    }
    bool flag = 0;
    for(int i=lineFrom;i<=lineTo;i++){
        //qDebug()<<textEdit->wordAtLineIndex(i,0);
        if(textEdit->wordAtLineIndex(i,0)!=""){
            flag =1;
            break;
        }
    }
    if(flag){//添加注释
        //qDebug()<<"添加注释";
        for(int i=lineFrom;i<=lineTo;i++){
            textEdit->insertAt (tr("//"), i, 0 );
        }
    }else{//取消注释
        //qDebug()<<"取消注释";
        for(int i=lineFrom;i<=lineTo;i++){
            textEdit->setSelection(i,0,i,2);
            textEdit->removeSelectedText();
        }
    }
}

/*
 * author zjm
 * description 树形目录打开文件
 * date 2019/9/6
 * */
void MainWindow::myTreeViewOpenFile(QModelIndex index){
    myqtreeview->selectFilePath="";
    //qDebug()<<index.data().toString();//文件名
    myqtreeview->selectFilePath = QString(index.data().toString()) + myqtreeview->selectFilePath;
    while(index.parent().data().toString()!=""){
        index=index.parent();
        //qDebug()<<index.data().toString();
        myqtreeview->selectFilePath = QString(index.data().toString()) + QString("/") + myqtreeview->selectFilePath;
    }
    qDebug()<<"打开文件"<<myqtreeview->selectFilePath;
    qDebug()<<curFile;
    qDebug()<<textEdit->isModified();
    if(curFile==""&&!textEdit->isModified()){//加载在本窗口(如果现在文件名是空且没被更新就本窗口打开，否则新窗口)
        loadFile(myqtreeview->selectFilePath);
    }else{//加载在新窗口
        MainWindow *newMainWindow = new MainWindow();
        newMainWindow->myqtreeview->reset();
        newMainWindow->fileDir = this->fileDir;
        newMainWindow->myqtreeview->model = new QDirModel;
        newMainWindow->myqtreeview->setModel(myqtreeview->model);
        newMainWindow->myqtreeview->setRootIndex(myqtreeview->model->index(fileDir));
        newMainWindow->loadFile(myqtreeview->selectFilePath);
        newMainWindow->show();
    }
}

/*
 * author zch
 * description debug窗口
 * date 2019/9/8
 * */
void MainWindow::debugSlot(){
    if(!mycompile())
        return;
    debugDialog.setProgram(curFile);
    debugDialog.setBreakpoints(breakpoints);
    debugDialog.initProperties();
    debugDialog.show();
    //debugDialog.showProperties(); //debug需要的信息
}

/*
 * author zch
 * description 单步执行标记
 * date 2019/9/9
 * */
void MainWindow::updateLineNumberSlot(int num){
    qDebug()<<"###### mianwindow: "<<num;
    clearMarker();
    textEdit->markerAdd(num - 1, 2);
}

/*
 * author zch
 * description 清除单步执行标记
 * date 2019/9/9
 * */
void MainWindow::clearMarker(){
    QString tmp = textEdit->text();
    int n = tmp.size();
    int idx,lne;
    textEdit->lineIndexFromPosition(n,&lne,&idx);
    for(int i = 0; i < lne + 2; i++)
        textEdit->markerDelete(i, 2);
}

/*
 * author lzy
 * description 行代码格式化
 * date 2019/9/9
 * */
void MainWindow::lineFormatting(int linenum){//传入行号
    QList<QString> keyword_table;
    keyword_table<<"+"<<"-"<<"*"<<"/"<<"("<<")"<<"="<<"<"<<">"<<","<<";";
    int sum_indexnum=0;//这一行总字符数

    sum_indexnum=textEdit->text(linenum).size();
    for(int indexnum=0;indexnum<sum_indexnum;indexnum++){//默认一行不超过200个字符
        textEdit->setSelection(linenum,indexnum,linenum,indexnum+1);
        for(int i=0;i<keyword_table.size();i++){
            if(textEdit->selectedText()=="+"){//+=、++单独处理
                textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);//先加左边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="+");//先看是否已有空格或+，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                textEdit->setSelection(linenum,indexnum+1,linenum,indexnum+2);//再加右边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="+"||textEdit->selectedText()=="=");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum+1);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                break;
            }
            else if(textEdit->selectedText()=="-"){//-=、--单独处理
                textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);//先加左边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="-");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                textEdit->setSelection(linenum,indexnum+1,linenum,indexnum+2);//再加右边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="-"||textEdit->selectedText()=="="||textEdit->selectedText()==">");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum+1);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                break;
            }
            else if(textEdit->selectedText()=="="){//+=,-=,*=,/=单独处理
                textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);//先加左边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="-"||textEdit->selectedText()=="+"||textEdit->selectedText()=="*"||textEdit->selectedText()=="/");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                textEdit->setSelection(linenum,indexnum+1,linenum,indexnum+2);//再加右边空格
                if(textEdit->selectedText()==" ");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum+1);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                break;
            }
            else if(textEdit->selectedText()==">"){//->单独处理
                textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);//先加左边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="-");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                textEdit->setSelection(linenum,indexnum+1,linenum,indexnum+2);//再加右边空格
                if(textEdit->selectedText()==" ");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum+1);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                break;
            }
            else if(textEdit->selectedText()=="*"){//*=单独处理
                textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);//先加左边空格
                if(textEdit->selectedText()==" ");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                textEdit->setSelection(linenum,indexnum+1,linenum,indexnum+2);//再加右边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="=");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum+1);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                break;
            }
            else if(textEdit->selectedText()=="/"){// /=单独处理
                textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);//先加左边空格
                if(textEdit->selectedText()==" ");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                textEdit->setSelection(linenum,indexnum+1,linenum,indexnum+2);//再加右边空格
                if(textEdit->selectedText()==" "||textEdit->selectedText()=="=");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum+1);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                break;
            }
            else if(textEdit->selectedText()==keyword_table[i]){
                textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);//先加左边空格
                if(textEdit->selectedText()==" ");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                textEdit->setSelection(linenum,indexnum+1,linenum,indexnum+2);//再加右边空格
                if(textEdit->selectedText()==" ");//先看是否已有空格，防止重复添加
                else {
                    textEdit->insertAt(" ",linenum,indexnum+1);
                    indexnum++;
                    sum_indexnum++;
                    //last_indexnum=indexnum;
                }
                break;
            }
        }
    }
    for(int i=sum_indexnum;i>=0;i--){
        textEdit->setSelection(linenum,i,linenum,i+1);
        if(textEdit->selectedText()==";"){
            this->textEdit->setCursorPosition(linenum, i+2);
            return;
        }
    }
}
/*
 * author lzy
 * description 整体代码格式化
 * date 2019/9/9
 * */
void MainWindow::Formatting_All(){
    int index_from,index_to,line_from,line_to;
    textEdit->selectAll();
    textEdit->getSelection(&line_from,&index_from,&line_to,&index_to);
    for(int i=0;i<line_to;i++){
        lineFormatting(i);
    }
    for(int i=0;i<line_to-1;i++){
        if(textEdit->text(i).at(textEdit->text(i).size()-2)=='{')//字符串末尾是回车，倒数第二才应该是{
            Line_Indent(i,1);
        else if(textEdit->text(i+1).at(textEdit->text(i+1).size()-2)=='}')
            Line_Indent(i,-1);
        else
            Line_Indent(i,0);
    }
}

void MainWindow::Enter_Formatting(int linenum){//行回车规范
    int sum_indexnum=0;//这一行总字符数
    sum_indexnum=textEdit->text(linenum).size();
    for(int i=0;i<sum_indexnum;i++){//顺便添加头文件规范功能
        textEdit->setSelection(linenum,i,linenum,i+1);
        if(textEdit->selectedText()=="#"){
            textEdit->setSelection(linenum,i+8,linenum,i+9);
            if(textEdit->selectedText()!=" ")
                this->textEdit->insertAt(tr(" "),linenum,i+8);
            this->textEdit->setCursorPosition(linenum+1, 0);
            return;
        }
    }

     QString temp=textEdit->text(linenum);
     qDebug()<<temp.size();
     //textEdit->setSelection(linenum,indexnum-1,linenum,indexnum);
     if(temp.at(temp.size()-2)=='{')//字符串末尾是回车，倒数第二应该是{
     {   Line_Indent(linenum,1);
        // if(textEdit->text(linenum+1).at(textEdit->text(linenum+1).size()-1)=='}')//这里是为了分辨是在创建{}还是统一检查规范
         this->textEdit->insert(tr("\n"));
         Line_Indent(linenum+1,-1);
         Line_Indent(linenum,1);
     }
     else
         Line_Indent(linenum,0);

}
/*
 * author lzy
 * description 行首缩进
 * date 2019/9/10
 * */
void MainWindow::Line_Indent(int linenum,int flag){//传入的是敲回车的行，要缩进的是下一行
    int sum_indexnum=0;//这一行总字符数
    int tab_num=0;//这一行的前置空格数
    int tab_num_new=0;//下一行需要添加的前置空格数，因为或许前面已经有了
    sum_indexnum=textEdit->text(linenum).size();
    QString temp=textEdit->text(linenum);
    tab_num=temp.count("\t");
    if(flag==1)
        tab_num++;//新的{要额外缩进
    if(flag==-1)
        tab_num--;//新的{要额外缩进
    tab_num_new=tab_num-textEdit->text(linenum+1).count("\t");
    qDebug()<<tab_num;
    for(int i=0;i<tab_num_new;i++){
        this->textEdit->insertAt(tr("\t"),linenum+1,i);
    }
    this->textEdit->setCursorPosition(linenum+1, tab_num);
}

/*
 * author zll
 * description 函数高亮
 * date 2019/9/8
 * */
void MainWindow::funcHighlighter(){
    QString text = textEdit -> text();
    int line=0,index=0;
    int sz = text.size();
    textEdit->lineIndexFromPosition(sz,&line,&index);
    textEdit->clearIndicatorRange(0,0,line,index,indicNum);
    QRegExp re("([_a-zA-Z]\\w*)(\\s*\\([^\\)]*\\))");
    int pos = text.indexOf(re);
    while(pos>=0){
        QRegExp re1("(\\w*)\\s*" + re.cap(1) );
        if( text.indexOf(re1)!= -1 && iconCPP.count(" " + re1.cap(1)  + " " )&& iconCPP.count(" " + re.cap(1) + " ") == 0 && re.cap(1) != "main"){
         //int func(int,int)
           // qDebug()<<re1.capturedTexts();
            line=0,index=0;
           // qDebug()<<re.capturedTexts();
            textEdit->lineIndexFromPosition(pos,&line,&index);
            textEdit->fillIndicatorRange(line,index,line,index+re.cap(1).size(),indicNum);
        }

        pos = text.indexOf(re,pos+re.matchedLength());
    }
}

/*
 * author zll
 * description 函数跳转
 * fix ctrl + 双击跳转
 * date 2019/9/8
 * */
void MainWindow::jumpDefination(int line,int index, Qt::KeyboardModifiers  state){
     textEdit->setCursorPosition(line,index + 2);
    if(QApplication::keyboardModifiers ()  == Qt::ControlModifier){
        QString name = textEdit->wordAtLineIndex(line,index),text = textEdit->text(),textLine = textEdit->text(line);
        QRegExp re;
        if(textLine.count("{") || textLine.count(";")==0 || textLine.indexOf(QRegExp(name+"\\s*\\(")) == -1)
            re.setPattern(name);
        else
            re .setPattern(name + "\\s*\\([^\\)]*\\)\\s*\\{");
        int pos = text.indexOf(re);

        if(pos != -1){
            int l,i;
            textEdit->lineIndexFromPosition(pos,&l,&i);
            textEdit->setCursorPosition(l,i);

        }

    }
}

/*
 * author zll zch
 * description 监听键盘 - 新需求子函数
 * date 2019/9/10
 * */
void MainWindow::recordPos(){
    if(isAnnotationHide){
        int line,index;
        textEdit->getCursorPosition(&line,&index);
        int pos = textEdit->positionFromLineIndex(line,index);
        if(!annotate.empty()) {
          for(int i=0;i<annotate.size();i++){
              if(pos < annotate[i].pos)
                  annotate[i].pos++;
          }
        }
    }
}

/*
 * author zll zch
 * description 新需求子函数
 * date 2019/9/10
 * */
void MainWindow::annotate_hide_and_show() {
    if(!isAnnotationHide){//隐藏
        int line = 0, index = 0, vectorSize = 0;
        QString text = textEdit->text();
        antt temp;

        QRegExp re_several("/\\*((?!\\*/).)*\\*/"), re_single("//[^\\n\\r]*");
        text = textEdit->text();
        int pos_several = text.indexOf(re_several), pos_single = text.indexOf(re_single);
        while (pos_several >= 0 || pos_single >= 0) {
            if (pos_several != -1 && (pos_several < pos_single || pos_single == -1)) {
                textEdit->lineIndexFromPosition(pos_several, &line, &index);
                int lineto, indexto;
                textEdit->lineIndexFromPosition(pos_several + re_several.matchedLength(), &lineto, &indexto);
                temp.pos = pos_several; temp.an = re_several.cap(0);
                if (lineto != line)
                    indexto += 2;

                annotate.push_back(temp); vectorSize++;
                qDebug() << "pos:" << pos_several;
                qDebug() << "delete annotate:" << annotate[vectorSize - 1].an;

                textEdit->setSelection(line, index, lineto, indexto);
                textEdit->removeSelectedText();

                QString textLine = textEdit->text(line);
                if (textLine.indexOf(QRegExp("^[\\n\\r]$")) != -1) {
                    //只要有用户输入的不在注释范围内的空白字符那么就不会删除这一行。
                    //换行符除非与/**/的结尾连在一起，不然也不会删除
                    annotate[vectorSize - 1].pos;
                    annotate[vectorSize - 1].an.append("\n");
                    textEdit->setSelection(line, 0, line, 1);
                    textEdit->removeSelectedText();
                }

                text = textEdit->text();
                pos_single = text.indexOf(re_single, pos_several);
                pos_several = text.indexOf(re_several, pos_several);
            }

            if (pos_single != -1 && (pos_several > pos_single || pos_several == -1)) {
                qDebug() << re_single.capturedTexts();
                textEdit->lineIndexFromPosition(pos_single, &line, &index);//获取行列
                //把注释放到vector变量里
                temp.pos = pos_single; temp.an = re_single.cap(0); annotate.push_back(temp); vectorSize++;
                qDebug() << "delete annotate:" << annotate[vectorSize - 1].an;
                qDebug() << "pos:" << pos_single;
                qDebug() << "vector pos:" << annotate[vectorSize - 1].pos;
                textEdit->setSelection(line, index, line, index + re_single.cap(0).size());
                textEdit->removeSelectedText();//选定删除（没有找到删除函数，但是剪切一样可以)
                QString textLine = textEdit->text(line);

                if (textLine.indexOf(QRegExp("^[\\n\\r]$")) != -1) {
                    //只要有用户输入的不在注释范围内的空白字符那么就不会删除这一行。
                    //换行符除非与/**/的结尾连在一起，不然也不会删除
                    annotate[vectorSize - 1].an += "\n";
                    textEdit->setSelection(line, 0, line, 1);
                    textEdit->removeSelectedText();
                }

                text = textEdit->text();//重新获取删除后的文本
                qDebug() << "text:" << text;
                pos_several = text.indexOf(re_several, pos_single);
                pos_single = text.indexOf(re_single, pos_single);//继续查找
            }
        }

        for (int i = 0; i < annotate.size(); i++) {
            qDebug() << "pos:" << annotate[i].pos << "  " << "annotate:" << annotate[i].an;
        }
        isAnnotationHide = true;
        return;
    }


    if (isAnnotationHide) {//显示
        if (!annotate.empty()) {
            int line, index;
            for (int i = annotate.size() - 1; i >= 0; i--) {
                qDebug() << "pos :" << annotate[i].pos;
                qDebug() << "textSize:" << textEdit->text().size();
                if (i != annotate.size() - 1) qDebug() << "text:" << textEdit->text(0, annotate[i].pos);
                textEdit->lineIndexFromPosition(annotate[i].pos, &line, &index);
                textEdit->insertAt(annotate[i].an, line, index);
            }
        }
        annotate.clear();
        isAnnotationHide = false;
        return;
    }
}
