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

    QDoubleValidator * lvalid = new QDoubleValidator(0.1, 1, 2, ui->lEdit);
    lvalid->setNotation(QDoubleValidator::StandardNotation);
    ui->lEdit->setValidator(lvalid);

    QDoubleValidator * Rvalid = new QDoubleValidator(0.1, 1, 2, ui->REdit);
    Rvalid->setNotation(QDoubleValidator::StandardNotation);
    ui->REdit->setValidator(Rvalid);

    QDoubleValidator * Mcvalid = new QDoubleValidator(0, 10, 1, ui->mcEdit);
    Mcvalid->setNotation(QDoubleValidator::StandardNotation);
    ui->mcEdit->setValidator(Mcvalid);

    QDoubleValidator * Mbvalid = new QDoubleValidator(0, 10, 1, ui->mbEdit);
    Mbvalid->setNotation(QDoubleValidator::StandardNotation);
    ui->mbEdit->setValidator(Mbvalid);

    QDoubleValidator * fvalid = new QDoubleValidator(0, 1, 2, ui->fEdit);
    fvalid->setNotation(QDoubleValidator::StandardNotation);
    ui->fEdit->setValidator(fvalid);

    QDoubleValidator * wvalid = new QDoubleValidator(0, 10, 2, ui->wEdit);
    wvalid->setNotation(QDoubleValidator::StandardNotation);
    ui->wEdit->setValidator(wvalid);

    QDoubleValidator * fivalid = new QDoubleValidator(0, 180, 2, ui->fiEdit);
    fivalid->setNotation(QDoubleValidator::StandardNotation);
    ui->fiEdit->setValidator(fivalid);

    QDoubleValidator * Pvalid = new QDoubleValidator(0, 20000000, 3, ui->PEdit);
    ui->PEdit->setValidator(Pvalid);

    ui->lEdit->setFocus();
    ui->statusBar->showMessage(tr("Эксперимент готов к запуску"));

    scene.addLine(-160, 0, 240, 0);
    scene.addLine(0, -160, 0, 160);

    rectLeft.setBrush(QBrush(Qt::white));
    rectRight.setBrush(QBrush(Qt::white));
    scene.addItem(&rectLeft);
    scene.addItem(&rectRight);
    scene.addItem(&wheel);
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

    double l=ui->lEdit->text().toDouble();
    if ( l < 0.1 || l > 1 ) {
        ui->lEdit->setText("0.1");
        l=0.1;
    }
    const double l_vis=l*150;

    double Gc=g*ui->mcEdit->text().toDouble();
    if ( Gc < 0 || Gc > 10*g ) {
        ui->mcEdit->setText("1");
        Gc=1;
    }

    double R=ui->REdit->text().toDouble();
    if ( R < 0.1 || R > 1 ) {
        ui->REdit->setText("0.1");
        R = 0.1;
    }

    double Gb=g*ui->mbEdit->text().toDouble();
    if ( Gb < 0 || Gb > 10*g ) {
        ui->mbEdit->setText("1");
        Gb=1;
    }

    double f=ui->fEdit->text().toDouble();
    if ( f < 0 || f > 1 ) {
        ui->fEdit->setText("0.1");
        f=0.1;
    }

    double w0=ui->wEdit->text().toDouble();
    if ( w0 < 0 || w0 > 10 ) {
        ui->wEdit->setText("1");
        w0=1;
    }

    double fi_razg=ui->fiEdit->text().toDouble();
    if ( fi_razg < 0 || fi_razg > 180 ) {
        ui->fiEdit->setText("30");
        fi_razg=30;
    }

    const double P=ui->PEdit->text().toDouble();

    double w=w0, wprev=0;
    double fi=0.01;
    const double dt=0.1;
    const double J=Gc*l*l/g/3 + Gb*R*R/g/2;
    const double b=Gc*l/2/J;
    line.setLine(-l_vis*sin(deg2rad(fi)), -l_vis*cos(deg2rad(fi)), 0, 0);
    wheel.setRect(-R*150, -R*150, R*150*2, R*150*2);
    rectLeft.setRect (R*150+20, -20, 20, 40);
    rectRight.setRect(R*150+40, -10, 40, 20);

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
            line.setLine(-l_vis*sin(deg2rad(fi)), -l_vis*cos(deg2rad(fi)), 0, 0);
            ui->graphicsView->centerOn(0, 0);
            angle_printed=fi;
        }
        wait();
    }

    //этап 2 - торможение до вертикали
    ui->statusBar->showMessage(tr("Торможение"));
    rectLeft.setRect(R*150, -20, 20, 40);
    rectRight.setRect(R*150+20, -10, 40, 20);
    const double d=P*R*f/J;
    for (uint k=0; fi<=180 && w>0; ++k) {
        w+=(b*sin(deg2rad(fi)) - d)*dt;
        fi+=(wprev+w)*dt/2;
        wprev=w;
        if (fi-angle_printed>dangle) {
            ui->fiCurEdit->setText(QString::number(fi));
            line.setLine(-l_vis*sin(deg2rad(fi)), -l_vis*cos(deg2rad(fi)), 0, 0);
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
                line.setLine(l_vis*sin(deg2rad(psi)), l_vis*cos(deg2rad(psi)), 0, 0);
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
