#ifndef CANTAB_H
#define CANTAB_H

#include <QWidget>
#include <QTimer>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class CanTab;
}

QT_END_NAMESPACE

class MainWindow;

class CanTab : public QWidget
{
    Q_OBJECT

public:
    explicit CanTab(QWidget *parent = 0);
    ~CanTab();

    void setMainWindow(MainWindow* mainWindow_);

private slots:
  void realtimeDataSlot();
  void reloadReport();

private:
    Ui::CanTab *ui;
    MainWindow *mainWindow;
    QTimer dataTimer;
};

#endif // CanTab_H
