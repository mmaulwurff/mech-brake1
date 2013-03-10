#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDoubleValidator>
#include <cmath>
#include <QPen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->showMessage(tr("Эксперимент готов к запуску"));
    scene.addEllipse(-50, -50, 100, 100);
    scene.addLine(-150, 0, 150, 0);
    scene.addLine(0, -150, 0, 150);
    rectLeft.setRect(60, -20, 20, 40);
    rectRight.setRect(80, -10, 40, 20);
    rectLeft.setBrush(QBrush(Qt::white));
    rectRight.setBrush(QBrush(Qt::white));
    scene.addItem(&rectLeft);
    scene.addItem(&rectRight);
    line.setPen(QPen(Qt::black, 3));
    scene.addItem(&line);
    ui->graphicsView->setScene(&scene);
    ui->graphicsView->show();
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
    ui->Start->setText(tr("Идет эксперимент..."));
    ui->Start->setEnabled(false);
    ui->restartButton->setEnabled(false);
    rectLeft.setRect(60, -20, 20, 40);
    rectRight.setRect(80, -10, 40, 20);
    const double l=ui->lEdit->text().toDouble();
    /*if ( 0==l ) {
        QMessageBox msgbox;
        msgbox.setText("invalid l");
        msgbox.exec();
        return;
    }*/

    const double Gc=g*ui->mcEdit->text().toDouble();

    const double R=ui->REdit->text().toDouble();

    const double Gb=g*ui->mbEdit->text().toDouble();

    const double f=ui->fEdit->text().toDouble();

    const double w0=ui->wEdit->text().toDouble();

    const double fi_razg=ui->fiEdit->text().toDouble();

    const double P=ui->PEdit->text().toDouble();

    double w=w0, wprev=0;
    double fi=0.01;
    const double dt=0.1;
    const double J=Gc*l*l/g/3 + Gb*R*R/g/2;
    const double b=Gc*l/2/J;
    line.setLine(-10*l*sin(deg2rad(fi)), -10*l*cos(deg2rad(fi)), 0, 0);

    ui->statusBar->showMessage(tr("Ускорение"));
    //разгон
    double angle_printed=fi;
    static const double dangle=1;
    for (uint k=0; fi<=fi_razg; ++k) {
        w+=b*sin(deg2rad(fi))*dt;
        fi+=(wprev+w)*dt/2;
        wprev=w;
        if (fi-angle_printed>dangle) {
            ui->fiCurEdit->setText(QString::number(fi));
            line.setLine(-10*l*sin(deg2rad(fi)), -10*l*cos(deg2rad(fi)), 0, 0);
            ui->graphicsView->centerOn(0, 0);
            angle_printed=fi;
        }
        wait();
    }

    //этап 2 - торможение до вертикали
    ui->statusBar->showMessage(tr("Торможение"));
    rectLeft.setRect(50, -20, 20, 40);
    rectRight.setRect(70, -10, 40, 20);
    const double d=P*R*f/J;
    for (uint k=0; fi<=180 && w>0; ++k) {
        w+=(b*sin(deg2rad(fi)) - d)*dt;
        fi+=(wprev+w)*dt/2;
        wprev=w;
        if (fi-angle_printed>dangle) {
            ui->fiCurEdit->setText(QString::number(fi));
            line.setLine(-10*l*sin(deg2rad(fi)), -10*l*cos(deg2rad(fi)), 0, 0);
            ui->graphicsView->centerOn(0, 0);
            angle_printed=fi;
        }
        wait();
    }

    angle_printed=0;
    if ( w>0 ) { //этап 3 - после вертикали
        double psi=0.01;
        for (uint k=0; w>0 && psi<=180; ++k) {
            w-=(b*sin(deg2rad(psi)) + d)*dt;
            psi+=(wprev+w)*dt/2;
            wprev=w;
            if (psi-angle_printed>dangle) {
                ui->fiCurEdit->setText(QString::number(180+psi));
                line.setLine(10*l*sin(deg2rad(psi)), 10*l*cos(deg2rad(psi)), 0, 0);
                ui->graphicsView->centerOn(0, 0);
                angle_printed=psi;
            }
            wait();
        }
        if (psi<=180)
            ui->statusBar->showMessage(tr("Эксперимент завершен."));
        else
            ui->statusBar->showMessage(tr("Эксперимент прерван."));
    } else
        ui->statusBar->showMessage(tr("Эксперимент прерван."));

    const double P_teor = (Gc*l + J*w0*w0/2)/R/f/(pi-deg2rad(fi_razg));
    ui->PTeorEdit->setText(QString::number(P_teor));
    const double err_p=100*qAbs(P-P_teor)/P_teor;
    ui->ErrEdit->setText(QString::number(err_p));
    if (err_p<5)
        ui->err5Label->setText(tr("не превышает 5%"));
    else
        ui->err5Label->setText(tr("превышает 5%"));
    ui->Start->setText(tr("Пуск"));
    ui->Start->setEnabled(true);
    ui->restartButton->setEnabled(true);
}

void MainWindow::on_exitButton_released()
{
    close();
}
