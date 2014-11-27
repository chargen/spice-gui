#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtNetwork>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

namespace Ui {
class SettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
        QString spinPackPath;

        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;

        QHostAddress liveSpikeAddress;
        quint16 liveSpikePort;
    };

    static SettingsDialog* getInstance();

    Settings settings() const;

private slots:
    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void browseSpinnakerPackage();

private:
    static SettingsDialog* settingsDialog;
    explicit SettingsDialog(QWidget *parent = 0);
    virtual ~SettingsDialog();

    void fillPortsParameters();
    void fillPortsInfo();
    void updateSettings();

    Ui::SettingsDialog *ui;
    Settings currentSettings;
    QIntValidator *intValidator;
    QDir* spinnakerPackageDir;
};

#endif // SETTINGSDIALOG_H
