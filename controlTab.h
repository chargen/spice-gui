#ifndef CONTROLTAB_H
#define CONTROLTAB_H

#include <QWidget>
#include <QTimer>

#include <type_traits>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class ControlTab;
}

QT_END_NAMESPACE

class MainWindow;

class ControlTab : public QWidget
{
    Q_OBJECT

public:
    explicit ControlTab(QWidget *parent = 0);
    ~ControlTab();

    void setMainWindow(MainWindow* mainWindow_);

private slots:
  void realtimeDataSlot();

  void sendData();

  void toggleMode();
  void setValue(int newValue);

private:
    Ui::ControlTab *ui;
    MainWindow *mainWindow;
    QTimer dataTimer;

    bool modeAutoTraj;

    double plotStartTime;
    double updateFrequency;
    double showPastTime;
    double windowWidth;
    double rightBlankTime;

    int processDataInterval;
    double prev_can_time_angle;
    double prev_error;
    double integral;
    double Kp;
    double Ki;
    double Kd;
};

#endif // CONTROLTAB_H
