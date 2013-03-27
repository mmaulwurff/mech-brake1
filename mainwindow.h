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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QEventLoop>
#include <QMessageBox>

const double pi=3.14;
const double g=9.81;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    double dt;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_action_triggered();

    void on_Start_released();

    void on_exitButton_released();

    void on_theoryButton_clicked();

    void on_autorsButton_clicked();

    void on_lEdit_editingFinished();

    void on_mcEdit_editingFinished();

    void on_REdit_editingFinished();

    void on_mbEdit_editingFinished();

    void on_fEdit_editingFinished();

    void on_wEdit_editingFinished();

    void on_fiEdit_editingFinished();

    void on_PEdit_editingFinished();

    private:
    Ui::MainWindow *ui;
    QGraphicsScene scene;
    QGraphicsLineItem line;
    QGraphicsLineItem razg_line;
    QGraphicsRectItem rectLeft;
    QGraphicsRectItem rectRight;
    QGraphicsEllipseItem wheel;

    QMessageBox msgbox; //окно предупреждения о некорректных значениях

    double deg2rad(double deg) const { return deg*2*pi/360; }
    double rad2deg(double rad) const { return rad*360/2/pi; }
    void wait() const;
};

#endif // MAINWINDOW_H
