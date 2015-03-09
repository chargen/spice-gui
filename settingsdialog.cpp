#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <fstream>
#include <sstream>

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QFileDialog>
#include <QRegExp>

QT_USE_NAMESPACE

SettingsDialog* SettingsDialog::settingsDialog = NULL;

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(ui->spynnakerCfgButton, SIGNAL(clicked()), this, SLOT(browseSpynnakerCfg()));

    intValidator = new QIntValidator(0, 4000000, this);

    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->applyButton, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(showPortInfo(int)));
    connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkCustomBaudRatePolicy(int)));

    fillPortsParameters();
    fillPortsInfo();

    // default values for SpiNNingCerebellum:
    ui->baudRateBox->setCurrentIndex(4);
    ui->baudRateBox->setCurrentText(QString::number(4000000));
    ui->flowControlBox->setCurrentIndex(1);
    ui->localEchoCheckBox->setChecked(true);
    ui->localEchoCheckBox->setEnabled(false);
    ui->liveSpikePortEdit->setText(QString::number(17895));
    QString user = qgetenv("USER");
    if(user.isEmpty())
        user = qgetenv("USERNAME");

    //ui->spynnakerCfgEdit->setText("/home/"+user+"/.spynnaker.cfg");
    //TODO: change it if you want to use the new/old SpiNNaker package release
    //ui->spynnakerCfgEdit->setText("/home/richtech/tools/spinnaker_package_jun14");
    //ui->spynnakerCfgEdit->setText("/home/sjentzsch/HBP/SpiNNaker/old/spinnaker_package_jun14");

    QString spinn_dirs = qgetenv("SPINN_DIRS");
    if(spinn_dirs.isEmpty())
        spinn_dirs = "/home/sjentzsch/HBP/SpiNNaker/workspace/spice-spinnaker-package";
    ui->spynnakerCfgEdit->setText(spinn_dirs);

    updateSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

SettingsDialog* SettingsDialog::getInstance()
{
    if(settingsDialog == NULL)
        settingsDialog = new SettingsDialog();
    return settingsDialog;
}

SettingsDialog::Settings SettingsDialog::settings() const
{
    return currentSettings;
}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx != -1) {
        QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
        ui->descriptionLabel->setText(tr("Description: %1").arg(list.at(1)));
        ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.at(2)));
        ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.at(3)));
        ui->locationLabel->setText(tr("Location: %1").arg(list.at(4)));
        ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.at(5)));
        ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.at(6)));
    }
}

void SettingsDialog::apply()
{
    updateSettings();
    hide();
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
}

void SettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(QStringLiteral("Custom"));

    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->parityBox->addItem(QStringLiteral("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(QStringLiteral("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(QStringLiteral("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(QStringLiteral("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(QStringLiteral("Space"), QSerialPort::SpaceParity);

    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(QStringLiteral("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(QStringLiteral("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(QStringLiteral("XON/XOFF"), QSerialPort::SoftwareControl);

    foreach(const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            ui->liveSpikeAddressBox->addItem(address.toString(), address.toString());
    }
}

void SettingsDialog::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    static const QString blankString = QObject::tr("N/A");
    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = 1; //info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }
}

void SettingsDialog::updateSettings()
{
    currentSettings.spynnakerCfgPath = ui->spynnakerCfgEdit->text();

    currentSettings.reportFilePath = "";
    currentSettings.applicationDataFilePath = "";
    currentSettings.isValidSpynnakerCfg = false;

    // TODO: comment me in again!
    /*std::ifstream spynnakerCfgFile(currentSettings.spynnakerCfgPath.toStdString());
    std::string strCfg;

    if(spynnakerCfgFile.good())
    {
        qDebug() << "Try parsing given spynnaker.cfg ...";
        spynnakerCfgFile.open(currentSettings.spynnakerCfgPath.toStdString());

        if(spynnakerCfgFile.is_open())
        {
            std::stringstream buffer;
            buffer << spynnakerCfgFile.rdbuf();
            strCfg = buffer.str();
            spynnakerCfgFile.close();

            QRegExp regParamInfo("^.*[Reports].*defaultReportFilePath\\s*=\\s*(\\S*)\\s*.*defaultApplicationDataFilePath\\s*=\\s*(\\S*)\\s*.*$");
            if(regParamInfo.indexIn(QString::fromStdString(strCfg)) >= 0)
            {
                currentSettings.reportFilePath = regParamInfo.cap(1);
                currentSettings.applicationDataFilePath = regParamInfo.cap(2);
                currentSettings.isValidSpynnakerCfg = true;

                qDebug() << "defaultReportFilePath =" << currentSettings.reportFilePath;
                qDebug() << "defaultApplicationDataFilePath =" << currentSettings.applicationDataFilePath;
                qDebug() << "Parse spynnaker.cfg: Success.";
            }
            else
                qDebug() << "Parse spynnaker.cfg: No regexp match.";
        }
    }*/

    currentSettings.name = ui->serialPortInfoListBox->currentText();

    if (ui->baudRateBox->currentIndex() == 4) {
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    } else {
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

    currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();

    currentSettings.liveSpikeAddress = QHostAddress(ui->liveSpikeAddressBox->itemData(ui->liveSpikeAddressBox->currentIndex()).toString());
    currentSettings.liveSpikePort = ui->liveSpikePortEdit->text().toUShort();
}

void SettingsDialog::browseSpynnakerCfg()
{
    // somehow Qt cannot be set to show hidden files by default. The user has to press STRG+H manually!

    QString path = QFileDialog::getOpenFileName(this, tr("Locate personal spynnaker.cfg (press STRG+H to show hidden files!)"), "/home/", tr("Config files (.*.cfg)"));

    if(!path.isNull())
    {
        ui->spynnakerCfgEdit->setText(path);
    }
}
