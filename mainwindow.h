#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showMessageStatusBar(const QString &text, int timeout = 0);
    void setEnabledConnect(bool enabled);
    void setEnabledDisconnect(bool enabled);

private slots:
       void about();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
