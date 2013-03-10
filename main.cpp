#include <QApplication>
#include <QTranslator>
#include "mainwindow.h"
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if ( QT_VERSION < 0x050000 )
        QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    MainWindow w;
    w.show();

    return a.exec();
}
