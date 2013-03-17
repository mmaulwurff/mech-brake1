	/*
	*This file is part of mech-brake1.
	*
	*mech-brake1 is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*mech-brake1 is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with mech-brake1. If not, see <http://www.gnu.org/licenses/>.
	*/

#include <QApplication>
#include <QTranslator>
#include "mainwindow.h"
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    #if ( QT_VERSION < 0x050000 )
        QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    #endif

    MainWindow w;
    w.show();

    return a.exec();
}
