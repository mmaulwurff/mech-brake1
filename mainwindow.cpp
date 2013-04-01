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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <QPen>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    msgbox.setIcon(QMessageBox::Warning);
    //оси x, y
    scene.addLine(-160, 0, 240, 0);
    scene.addLine(0, -160, 0, 160);
    //два прямоугольника колодки
    rectLeft.setBrush(QBrush(Qt::white));
    rectRight.setBrush(QBrush(Qt::white));
    //стержень
    line.setPen(QPen(Qt::black, 3));
    //угол разгона
    razg_line.setPen(QPen(Qt::red, 1));
    scene.addItem(&rectLeft);
    scene.addItem(&rectRight);
    scene.addItem(&wheel);
    scene.addItem(&razg_line);
    scene.addItem(&line);
    ui->graphicsView->setScene(&scene);
    ui->graphicsView->show();

    QSettings sett(QDir::currentPath()+"/mech-brake1.ini", QSettings::IniFormat);
    //QSettings::setDefaultFormat(QSettings::IniFormat);
    ui->lEdit ->setText(sett.value("l", "0.50").toString());
    ui->mcEdit->setText(sett.value("mc", "5.00").toString());
    ui->REdit ->setText(sett.value("R", "0.50").toString());
    ui->mbEdit->setText(sett.value("mb", "5.00").toString());
    ui->fEdit ->setText(sett.value("f", "0.30").toString());
    ui->wEdit ->setText(sett.value("w", "5.00").toString());
    ui->fiEdit->setText(sett.value("fi", "45").toString());
    ui->PEdit ->setText(sett.value("P", "50").toString());

    on_lEdit_editingFinished();
    on_REdit_editingFinished();
    on_fiEdit_editingFinished();
    ui->lEdit->setFocus();
}

MainWindow::~MainWindow()
{
    QSettings sett(QDir::currentPath()+"/mech-brake1.ini", QSettings::IniFormat);
    sett.setValue("l", "0.50");
    sett.setValue("mc", "5.00");
    sett.setValue("R", "0.50");
    sett.setValue("mb", "5.00");
    sett.setValue("f", "0.30");
    sett.setValue("w", "5.00");
    sett.setValue("fi", "45");
    sett.setValue("P", "50");
    delete ui;
}

void MainWindow::on_action_triggered()
{
    close();
}

void MainWindow::on_Start_released()
{
    //доступные для записи
    //lEdit  - поле ввода длины стержня
    //mcEdit - поле ввода массы стержня
    //REdit  - поле ввода радиуса барабана
    //mbEdit - поле ввода массы барабана
    //fEdit  - поле ввода коэффициента трения
    //wEdit  - поле ввода начальной угловой скорости
    //fiEdit - поле ввода угла разгона
    //PEdit  - поле ввода расчётной силы нажатия колодки
    //только для чтения:
    //fiCurEdit - текущий угол поворота
    //PTeorEdit - теоретическая сила нажатия колодки
    //ErrEdit   - ошибка расчёта
    ui->fiCurEdit->clear();
    ui->PTeorEdit->clear();
    ui->ErrEdit  ->clear();
    ui->percEdit ->clear();

    ui->lEdit ->setEnabled(false);
    ui->mcEdit->setEnabled(false);
    ui->REdit ->setEnabled(false);
    ui->mbEdit->setEnabled(false);
    ui->fEdit ->setEnabled(false);
    ui->wEdit ->setEnabled(false);
    ui->fiEdit->setEnabled(false);
    ui->PEdit ->setEnabled(false);

    const double l=ui->lEdit->text().toDouble(); //длина стержня, м
    const double l_vis=l*150; //длина стрежня для отображения на экране
    const double Gc=g*ui->mcEdit->text().toDouble(); //вес стержня, кг
    const double R=ui->REdit->text().toDouble(); //радиус барабана, м
    const double Gb=g*ui->mbEdit->text().toDouble(); //вес барабана, кг
    const double f=ui->fEdit->text().toDouble(); //коэффициент трения, безразмерный
    const double w0=ui->wEdit->text().toDouble(); //начальная угловая скорость, рад/с
    const double fi_razg=deg2rad(ui->fiEdit->text().toDouble()); //угол разгона, рад

    razg_line.setLine(-160*sin(fi_razg), -160*cos(fi_razg), 0, 0);
    line.setLine(0, -l*150, 0, 0);
    wheel.setRect(-R*150, -R*150, R*150*2, R*150*2);
    rectLeft.setRect (R*150+20, -20, 20, 40);
    rectRight.setRect(R*150+40, -10, 40, 20);

    double w=w0; //текущая угловая скорость (w_k), рад/с
    double wprev=w0; //предыдущая угловая скорость (w_k-1), рад/с
    double fi=0.0; //текущий угол поворота, рад
    const double J=Gc*l*l/g/3 + Gb*R*R/g/2;
    const double b=Gc*l/2/J;

    ui->Start->setText(tr("Идет эксперимент..."));
    ui->Start->setEnabled(false);

    //разгон
    for (uint k=0; fi<=fi_razg; ++k) {
        w+=b*sin(fi)*dt;
        fi+=(wprev+w)*dt/2;
        wprev=w;
        if ( 0==k%10 ) {
            ui->fiCurEdit->setText(QString::number(int(rad2deg(fi))));
            line.setLine(-l_vis*sin(fi), -l_vis*cos(fi), 0, 0);
        }
        wait();
    }

    //этап 2 - торможение до вертикали
    rectLeft.setRect (R*150,    -20, 20, 40);
    rectRight.setRect(R*150+20, -10, 40, 20);

    const double P=ui->PEdit->text().toDouble();
    const double d=P*R*f/J;
    for (uint k=0; fi<=pi && w>0; ++k) {
        w+=(b*sin(fi) - d)*dt;
        fi+=(wprev+w)*dt/2;
        wprev=w;
        if ( 0==k%10 ) {
            ui->fiCurEdit->setText(QString::number(int(rad2deg(fi))));
            line.setLine(-l_vis*sin(fi), -l_vis*cos(fi), 0, 0);
        }
        wait();
    }

    if ( w>0 ) { //этап 3 - после вертикали
        double psi=0.0;
        for (uint k=0; w>0; ++k) {
            w-=(b*sin(psi) + d)*dt;
            psi+=(wprev+w)*dt/2;
            wprev=w;
            if ( 0==k%10 ) {
                ui->fiCurEdit->setText(QString::number(int(rad2deg(pi+psi))));
                line.setLine(l_vis*sin(psi), l_vis*cos(psi), 0, 0);
            }
            if ( psi>=pi ) {
                msgbox.setText(tr("Эксперимент прерван: стержень совершил оборот."));
                msgbox.exec();
                break;
            }
            wait();
        }
    }

    const double P_teor = (Gc*l + J*w0*w0/2)/R/f/(pi-fi_razg);
    ui->PTeorEdit->setText(QString::number(P_teor));
    const double err_p=100 * ( P - P_teor ) / P_teor;
    ui->ErrEdit->setText(QString::number(err_p));
    if ( qAbs(err_p) < 5 )
        ui->percEdit->setText(tr("не превышает 5%"));
    else
        ui->percEdit->setText(tr("превышает 5%"));
    ui->Start->setText(tr("Пуск"));
    ui->Start ->setEnabled(true);
    ui->lEdit ->setEnabled(true);
    ui->mcEdit->setEnabled(true);
    ui->REdit ->setEnabled(true);
    ui->mbEdit->setEnabled(true);
    ui->fEdit ->setEnabled(true);
    ui->wEdit ->setEnabled(true);
    ui->fiEdit->setEnabled(true);
    ui->PEdit ->setEnabled(true);
}

void MainWindow::on_exitButton_released()
{
    close();
}

void MainWindow::on_theoryButton_clicked()
{
    const QString path="file:///"+QDir::currentPath()+"/theory.pdf";
    if ( !QDesktopServices::openUrl(QUrl(path)) ) {
        msgbox.setText(tr("Не удаётся открыть файл с теорией: ")+path);
        msgbox.exec();
    }
}

void MainWindow::wait() const
{
    QEventLoop loop;
    QTimer::singleShot(1000*ui->intenSlider->value()*dt, &loop, SLOT(quit()));
    loop.exec();
}

void MainWindow::on_autorsButton_clicked()
{
    QFile file("authors.txt");
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        msgbox.setText(tr("Не найден файл authors.txt"));
        return;
    }
    QMessageBox msgbox;
    QTextStream in(&file);
    QString str=in.readAll();
    msgbox.setText(str);
    msgbox.exec();
}

void MainWindow::on_lEdit_editingFinished()
{
    bool ok;
    const double l=ui->lEdit->text().toDouble(&ok);
    if ( !ok || l < 0.1 || l > 1 ) {
        msgbox.setText(tr("Некорректная длина стержня."));
        msgbox.exec();
        ui->lEdit->setFocus();
    } else
        line.setLine(0, -l*150, 0, 0);
}

void MainWindow::on_mcEdit_editingFinished()
{
    bool ok;
    const double Gc=g*ui->mcEdit->text().toDouble(&ok);
    if ( !ok || Gc < 0 || Gc > 10*g ) {
        msgbox.setText(tr("Некорректная масса стержня."));
        msgbox.exec();
        ui->mcEdit->setFocus();
    }
}

void MainWindow::on_REdit_editingFinished()
{
    bool ok;
    const double R=ui->REdit->text().toDouble(&ok);
    if ( !ok || R < 0.1 || R > ui->lEdit->text().toDouble() ) {
        msgbox.setText(tr("Некорректный радиус тормозного барабана."));
        msgbox.exec();
        ui->REdit->setFocus();
    } else {
        wheel.setRect(-R*150, -R*150, R*150*2, R*150*2);
        rectLeft.setRect (R*150+20, -20, 20, 40);
        rectRight.setRect(R*150+40, -10, 40, 20);
    }
}

void MainWindow::on_mbEdit_editingFinished()
{
    bool ok;
    const double Gb=g*ui->mbEdit->text().toDouble(&ok);
    if ( !ok || Gb < 0 || Gb > 10*g ) {
        msgbox.setText(tr("Некорректная масса тормозного барабана."));
        msgbox.exec();
        ui->mbEdit->setFocus();
    }
}

void MainWindow::on_fEdit_editingFinished()
{
    bool ok;
    const double f=ui->fEdit->text().toDouble(&ok);
    if ( !ok || f < 0.1 || f > 0.9 ) {
        msgbox.setText(tr("Некорректный коэффициент трения."));
        msgbox.exec();
        ui->fEdit->setFocus();
    }
}

void MainWindow::on_wEdit_editingFinished()
{
    bool ok;
    const double w0=ui->wEdit->text().toDouble(&ok);
    if ( !ok || w0 < 0.1 || w0 > 10 ) {
        msgbox.setText(tr("Некорректная начальная угловая скорость."));
        msgbox.exec();
        ui->wEdit->setFocus();
    }
}

void MainWindow::on_fiEdit_editingFinished()
{
    bool ok;
    const double fi_razg=ui->fiEdit->text().toDouble(&ok);
    if ( !ok || fi_razg < 0 || fi_razg > 180 ) {
        ui->fiEdit->setFocus();
        msgbox.setText(tr("Некорректный угол разгона."));
        msgbox.exec();
    } else
        razg_line.setLine(-160*sin(deg2rad(fi_razg)), -160*cos(deg2rad(fi_razg)), 0, 0);
}

void MainWindow::on_PEdit_editingFinished()
{
    bool ok;
    const double p_teor=ui->PEdit->text().toDouble(&ok);
    if ( !ok || p_teor < 0 ) {
        msgbox.setText(tr("Некорректная теоретическая сила нажатия."));
        msgbox.exec();
        ui->PEdit->setFocus();
    }
}

void MainWindow::on_intenSlider_valueChanged(int value)
{
    ui->intenLabel->setText(tr("Интенсивность визуализации: ")+QString::number(6-value));
}
