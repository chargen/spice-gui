#ifndef CONTROLTAB_H
#define CONTROLTAB_H

#include <QWidget>
#include <QTimer>

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

  void setMotor();

private:
    Ui::ControlTab *ui;
    MainWindow *mainWindow;
    QTimer dataTimer;

    double plotStartTime;
    double updateFrequency;
    double showPastTime;
    double windowWidth;
    double rightBlankTime;
};

#endif // CONTROLTAB_H
