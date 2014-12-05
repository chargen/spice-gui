#ifndef PLOTTAB_H
#define PLOTTAB_H

#include <QWidget>
#include <QTimer>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class PlotTab;
}

QT_END_NAMESPACE

class MainWindow;

class PlotTab : public QWidget
{
    Q_OBJECT

public:
    explicit PlotTab(QWidget *parent = 0);
    ~PlotTab();

    void setMainWindow(MainWindow* mainWindow_);

private slots:
  void realtimeDataSlot();
  void reloadReport();
  void savePlot();
  void setMotors();

private:
    Ui::PlotTab *ui;
    MainWindow *mainWindow;
    QTimer dataTimer;
};

#endif // PLOTTAB_H
