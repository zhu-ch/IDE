#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDialog>
#include <QLabel>
#include <QShortcut>
#include <QTextCodec>
#include <QSplitter>
#include <QDirModel>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
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

#include <Qsci/qscilexercpp.h>
#include<Qsci/qsciscintilla.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qsciapis.h>

#include <stdio.h>
#include <vector>
#include <set>

#include "finddialog.h"
#include "replacedialog.h"
#include "myqtreeview.h"
#include "debugdialog.h"

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
    void keyPressSiganl_puncComplete(int);

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

    bool mycompile();
    void myrun();
    void compile_run();
    void all_compile();
    QList<QString> findHead();
    void all_run();
    void debugSlot();

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

    void funcHighlighter();
    void jumpDefination(int line,int index, Qt::KeyboardModifiers state);

    void Formatting_All();

    void handlePuncComplete(int);
    void Annotation();

    void myTreeViewOpenFile(QModelIndex);

    void updateLineNumberSlot(int);
    void clearMarker();

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
    void lineFormatting(int linenumber);//行代码格式化（通过；触发）
    void Enter_Formatting(int linenum);
    void Line_Indent(int linenum,int flag);

    QsciScintilla *textEdit;            //代码编辑框
    //QFrame *colorFrame;                 //样式 大小
    //QLineEdit *fontLineEdit;            //颜色
    //QscilexerCppAttach *textLexer;
    QsciLexer *textLexer;               //语法分析器
    QTextEdit *LogText;                 //控制台
    QString curFile;                    //当前打开文件的路径

    //树形目录部分
    QString fileDir;//当前树形目录根目录
    QSplitter *splitter;
    //QDirModel *model;
    MyQTreeView *myqtreeview;

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
    QAction *annotation;//添加注释
    QAction *allFormattingAct;//整体注释

    //帮助
    QMenu *helpMenu;
    QAction *aboutAct;
    QAction *aboutQtAct;


    //编译运行
    QMenu *compileMenu;
    QAction *compileAct;//编译
    QAction *runAct;//运行
    QAction *CompileRunAct;
    QAction *allCompileAct;
    QAction *allRunAct;
    QAction *debugAct;

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
    QVBoxLayout *mainLayoutV;    //格子布局
    QHBoxLayout *mainLayoutH;

    //变量重命名相关：因为没法实现匿名函数所以需要的变量
    QString variableName;
    QLineEdit *lineEdit;

    //监听按键
    MyKeyPressEater *keyPressEater;

    //光标位置
    int cursorLine;
    int cursorIndex;

    //状态栏设置标签
    QLabel* first_statusLabel; //声明两个标签对象，用于显示状态信息
    QLabel* second_statusLabel;
    void init_statusBar(); //初始化状态栏

    //多行注释
    int lineFrom;
    int indexFrom;

    int lineTo;
    int indexTo;

    //debug
    std::vector<int> breakpoints;//断点集合
    DebugDialog debugDialog;//debug窗口

    //函数高亮
    int indicNum;
    QString iconCPP;//cpp关键字字符合集
};

#endif
