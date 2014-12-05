#ifndef SERIALTAB_H
#define SERIALTAB_H

#include <QtCore/QtGlobal>

#include <QWidget>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class SerialTab;
}

QT_END_NAMESPACE

class MainWindow;
class Console;

class SerialTab : public QWidget
{
    Q_OBJECT

public:
    explicit SerialTab(QWidget *parent = 0);
    ~SerialTab();

    void setMainWindow(MainWindow* mainWindow_);

private slots:
    void writeData(const QByteArray &data);
    void readData();
    void clear();

private:
    Ui::SerialTab *ui;
    MainWindow *mainWindow;
    Console *console;
};

#endif // SERIALTAB_H
