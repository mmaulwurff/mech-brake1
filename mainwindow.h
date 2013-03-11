#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QEventLoop>

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

    void on_restartButton_clicked();

    private:
    Ui::MainWindow *ui;
    QGraphicsScene scene;
    QGraphicsLineItem line;
    QGraphicsRectItem rectLeft;
    QGraphicsRectItem rectRight;
    QGraphicsEllipseItem wheel;
    double deg2rad(double deg) const { return deg*2*pi/360; }
    double rad2deg(double rad) const { return rad*360/2/pi; }
    void wait() const;
};

#endif // MAINWINDOW_H
