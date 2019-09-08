CONFIG      += release qscintilla2

macx {
    QMAKE_POST_LINK = install_name_tool -change libqscintilla2.12.dylib $$[QT_INSTALL_LIBS]/libqscintilla2.12.dylib $(TARGET)
}

HEADERS      = mainwindow.h \
    FindDialog.h \
    ReplaceDialog.h \
    myqtreeview.h \
    runthread.h \
    debugdialog.h
SOURCES      = main.cpp mainwindow.cpp \
    FindDialog.cpp \
    ReplaceDialog.cpp \
    myqtreeview.cpp \
    runthread.cpp \
    debugdialog.cpp
RESOURCES    = application.qrc

#以下为新增
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_10_1_MinGW_32bit-Release/release/ -lqscintilla2_qt5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_10_1_MinGW_32bit-Release/debug/ -lqscintilla2_qt5
else:unix: LIBS += -L$$PWD/../../QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_10_1_MinGW_32bit-Release/ -lqscintilla2_qt5

INCLUDEPATH += $$PWD/../../QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_10_1_MinGW_32bit-Release/releas
DEPENDPATH += $$PWD/../../QScintilla_gpl-2.11.2/build-qscintilla-Desktop_Qt_5_10_1_MinGW_32bit-Release/release

INCLUDEPATH += G:\QScintilla_gpl-2.11.2\Qt4Qt5
