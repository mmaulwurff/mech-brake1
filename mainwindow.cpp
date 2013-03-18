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
#include <QMessageBox>
#include <cmath>
#include <QPen>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //оси x, y
    scene.addLine(-160, 0, 240, 0);
    scene.addLine(0, -160, 0, 160);

    rectLeft.setBrush(QBrush(Qt::white));
    rectRight.setBrush(QBrush(Qt::white));
    scene.addItem(&rectLeft);
    scene.addItem(&rectRight);
    scene.addItem(&wheel);
    line.setPen(QPen(Qt::black, 3));
    razg_line.setPen(QPen(Qt::red, 1));
    scene.addItem(&line);
    scene.addItem(&razg_line);
    ui->graphicsView->setScene(&scene);
    ui->graphicsView->show();

    ui->lEdit->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_triggered()
{
    close();
}

void MainWindow::on_Start_released()
{
    ui->fiCurEdit->clear();
    ui->PTeorEdit->clear();
    ui->ErrEdit  ->clear();

    ui->lEdit ->setEnabled(false);
    ui->mcEdit->setEnabled(false);
    ui->REdit ->setEnabled(false);
    ui->mbEdit->setEnabled(false);
    ui->fEdit ->setEnabled(false);
    ui->wEdit ->setEnabled(false);
    ui->fiEdit->setEnabled(false);
    ui->PEdit ->setEnabled(false);

    const double l=ui->lEdit->text().toDouble();
    const double l_vis=l*150;
    const double Gc=g*ui->mcEdit->text().toDouble();
    const double R=ui->REdit->text().toDouble();
    const double Gb=g*ui->mbEdit->text().toDouble();
    const double f=ui->fEdit->text().toDouble();
    const double w0=ui->wEdit->text().toDouble();
    const double fi_razg=deg2rad(ui->fiEdit->text().toDouble());

    line.setLine(0, -l*150, 0, 0);
    wheel.setRect(-R*150, -R*150, R*150*2, R*150*2);
    razg_line.setLine(-160*sin(fi_razg), -160*cos(fi_razg), 0, 0);

    double w=w0, wprev=w0;
    double fi=0.01;
    dt=0.001;
    const double J=Gc*l*l/g/3 + Gb*R*R/g/2;
    const double b=Gc*l/2/J;

    ui->Start->setText(tr("Идет эксперимент..."));
    ui->Start->setEnabled(false);

    rectLeft.setRect (R*150+20, -20, 20, 40);
    rectRight.setRect(R*150+40, -10, 40, 20);

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
        double psi=0.01;
        for (uint k=0; w>0 && psi<=pi; ++k) {
            w-=(b*sin(psi) + d)*dt;
            psi+=(wprev+w)*dt/2;
            wprev=w;
            if ( 0==k%10 ) {
                ui->fiCurEdit->setText(QString::number(int(rad2deg(pi+psi))));
                line.setLine(l_vis*sin(psi), l_vis*cos(psi), 0, 0);
            }
            wait();
        }
    }

    const double P_teor = (Gc*l + J*w0*w0/2)/R/f/(pi-fi_razg);
    ui->PTeorEdit->setText(QString::number(P_teor));
    const double err_p=100*qAbs(P-P_teor)/P_teor;
    ui->ErrEdit->setText(QString::number(err_p));
    if ( err_p < 5 )
        ui->err5Label->setText(tr("не превышает 5%"));
    else
        ui->err5Label->setText(tr("превышает 5%"));
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
    if ( !QDesktopServices::openUrl(QUrl(QDir::currentPath()+"/Theory.pdf") )) {
        QMessageBox msgbox;
        msgbox.setText(tr("Не удаётся открыть файл с теорией."));
        msgbox.exec();
    }
}

void MainWindow::wait() const
{
    QEventLoop loop;
    QTimer::singleShot(ui->intenSlider->value()*dt, &loop, SLOT(quit()));
    loop.exec();
}

void MainWindow::on_autorsButton_clicked()
{
    QMessageBox msgbox;
    msgbox.setText(tr("Авторы:\nРуководитель: Крамаренко Николай Владимирович\nИсполнитель: Кромм Александр Юрьевич"));
    msgbox.exec();
}

void MainWindow::on_lEdit_editingFinished()
{
    const double l=ui->lEdit->text().toDouble();
    if ( l < 0.1 || l > 1 ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректная длина стержня."));
        msgbox.exec();
        ui->lEdit->clear();
        ui->lEdit->setFocus();
    } else {
        line.setLine(0, -l*150, 0, 0);
        ui->mcEdit->setFocus();
    }
}

void MainWindow::on_mcEdit_editingFinished()
{
    const double Gc=g*ui->mcEdit->text().toDouble();
    if ( Gc < 0 || Gc > 10*g ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректная масса стержня."));
        msgbox.exec();
        ui->mcEdit->clear();
        ui->mcEdit->setFocus();
    } else
        ui->REdit->setFocus();
}

void MainWindow::on_REdit_editingFinished()
{
    const double R=ui->REdit->text().toDouble();
    if ( R < 0.1 || R > 1 ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректный радиус тормозного барабана."));
        msgbox.exec();
        ui->REdit->clear();
        ui->REdit->setFocus();
    } else {
        wheel.setRect(-R*150, -R*150, R*150*2, R*150*2);
        ui->mbEdit->setFocus();
    }
}

void MainWindow::on_mbEdit_editingFinished()
{
    const double Gb=g*ui->mbEdit->text().toDouble();
    if ( Gb < 0 || Gb > 10*g ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректная масса тормозного барабана."));
        msgbox.exec();
        ui->mbEdit->clear();
        ui->mbEdit->setFocus();
    } else
        ui->fEdit->setFocus();
}

void MainWindow::on_fEdit_editingFinished()
{
    const double f=ui->fEdit->text().toDouble();
    if ( f < 0 || f > 0.9 ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректный коэффициент трения."));
        msgbox.exec();
        ui->fEdit->clear();
        ui->fEdit->setFocus();
    } else
        ui->wEdit->setFocus();
}

void MainWindow::on_wEdit_editingFinished()
{
    const double w0=ui->wEdit->text().toDouble();
    if ( w0 < 0 || w0 > 10 ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректная начальная угловая скорость."));
        msgbox.exec();
        ui->wEdit->clear();
        ui->wEdit->setFocus();
    } else
        ui->fiEdit->setFocus();
}

void MainWindow::on_fiEdit_editingFinished()
{
    const double fi_razg=deg2rad(ui->fiEdit->text().toDouble());
    if ( fi_razg < 0 || fi_razg > 180 ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректный угол разгона."));
        msgbox.exec();
        ui->fiEdit->clear();
        ui->fiEdit->setFocus();
    } else {
        ui->PEdit->setFocus();
        razg_line.setLine(-160*sin(fi_razg), -160*cos(fi_razg), 0, 0);
    }
}

void MainWindow::on_PEdit_editingFinished()
{
    bool convert_ok;
    ui->PEdit->text().toDouble(&convert_ok);
    if ( !convert_ok ) {
        QMessageBox msgbox;
        msgbox.setText(tr("Некорректная теоретическая сила нажатия."));
        msgbox.exec();
        ui->PEdit->clear();
        ui->PEdit->setFocus();
    } else
        ui->Start->setFocus();
}
