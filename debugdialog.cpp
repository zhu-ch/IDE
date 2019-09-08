#include "debugdialog.h"

DebugDialog::DebugDialog(){
    initWidgets();
    bindSignals();

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

void DebugDialog::initWidgets(){
    //初始化控件
    nextBtn = new QPushButton("单步跳过");
    stepBtn = new QPushButton("单步进入");
    continueBtn = new QPushButton("继续执行");
    runBtn = new QPushButton("开始调试");
    quitBtn = new QPushButton("结束调试");
    addVarBtn = new QPushButton("添加查看");
    inputVar = new QLineEdit();
    printArea = new QTextEdit();

    //输出部分只读
    printArea->setReadOnly(true);

    //布局
    layout = new QGridLayout();

    layout->setRowStretch(0,3);
    layout->setRowStretch(1,14);
    layout->setColumnStretch(0,20);
    layout->setColumnMinimumWidth(1,3);
    layout->setSpacing(10);

    layout->addWidget(inputVar,0,0);
    layout->addWidget(addVarBtn,0,1);
    layout->addWidget(printArea,1,0);

    vbLayout = new QVBoxLayout();
    vbLayout->addWidget(runBtn);
    vbLayout->addWidget(stepBtn);
    vbLayout->addWidget(nextBtn);
    vbLayout->addWidget(continueBtn);
    vbLayout->addWidget(quitBtn);
    //vbLayout->setSpacing(5);

    layout->addLayout(vbLayout,1,1);

    setMinimumHeight(300);
    setMinimumWidth(600);
    setLayout(layout);
}

void DebugDialog::initProperties(){
    inputVar->setText("");
    printArea->setText("");
    var.clear();
}

void DebugDialog::bindSignals(){
    connect(addVarBtn, &QPushButton::clicked, this, &DebugDialog::slotAddVar);
    connect(runBtn, &QPushButton::clicked, this, &DebugDialog::slotRun);
    connect(stepBtn, &QPushButton::clicked, this, &DebugDialog::slotStep);
    connect(nextBtn, &QPushButton::clicked, this, &DebugDialog::slotNext);
    connect(quitBtn, &QPushButton::clicked, this, &DebugDialog::slotQuit);
    connect(continueBtn, &QPushButton::clicked, this, &DebugDialog::slotContinue);
}

void DebugDialog::setProgram(QString p){
    this->program = p;
    QString name;
    if(p == "")
        name = "untitled";
    else
        name = p;
    setWindowTitle("Debug - " + name);
}

void DebugDialog::setBreakpoints(std::vector<int> b){
    this->breakpoints = b;
}

void DebugDialog::slotNext(){

}

void DebugDialog::slotStep(){

}

void DebugDialog::slotContinue(){

}

void DebugDialog::slotRun(){
    //先确定编译完成

}

void DebugDialog::slotQuit(){

}

void DebugDialog::slotAddVar(){
    QString toAddVar = inputVar->text();
    if(toAddVar == ""){
        QMessageBox msg(NULL);

        msg.setWindowTitle("Add Watch");
        msg.setText("No input");
        msg.setIcon(QMessageBox::Information);
        msg.setStandardButtons(QMessageBox::Ok);

        msg.setWindowFlags(Qt::WindowStaysOnTopHint);//置顶
        msg.setWindowFlags(msg.windowFlags() &~ Qt::WindowMinMaxButtonsHint);//禁用最大化最小化
        msg.exec();
    }
    else{
        var.push_back(toAddVar);
        printArea->append("<p><font color = blue>Add Watch: " + toAddVar + "</font></p>");
        QScrollBar *scrollbar = printArea->verticalScrollBar();
        if (scrollbar)
            scrollbar->setSliderPosition(scrollbar->maximum());
        inputVar->setText("");
    }
}

void DebugDialog::showProperties(){
    qDebug()<<"debug dialog properties";
    qDebug()<<"program: "<<program;
    for(std::vector<int>::iterator it = breakpoints.begin(); it != breakpoints.end(); ){
        qDebug()<<*it;
        it++;
    }
    qDebug()<<"end of breakpoints";
    for(std::vector<QString>::iterator it = var.begin(); it != var.end(); ){
        qDebug()<<*it;
        it++;
    }
    qDebug()<<"end of var";
}
