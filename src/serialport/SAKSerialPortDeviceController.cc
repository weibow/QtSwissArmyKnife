/*
 * Copyright (C) 2018-2019 wuuhii. All rights reserved.
 *
 * The file is encoding with utf-8 (with BOM). It is a part of QtSwissArmyKnife
 * project. The project is a open source project, you can get the source from:
 *     https://github.com/wuuhii/QtSwissArmyKnife
 *     https://gitee.com/wuuhii/QtSwissArmyKnife
 *
 * If you want to know more about the project, please join our QQ group(952218522).
 * In addition, the email address of the project author is wuuhii@outlook.com.
 */
#include <QList>
#include <QMetaEnum>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QDebug>
#include <QDialog>

#include "SAKGlobal.hh"
#include "SAKSerialPortDeviceController.hh"
#include "ui_SAKSerialPortDeviceController.h"
SAKSerialPortDeviceController::SAKSerialPortDeviceController(QWidget *parent)
    :QWidget (parent)
    ,ui (new Ui::SAKSerialPortDeviceController)
{
    ui->setupUi(this);

    comboBoxSerialports      = ui->comboBoxSerialports;
    comboBoxBaudrate         = ui->comboBoxBaudrate;
    comboBoxDatabits         = ui->comboBoxDatabits;
    comboBoxStopbits         = ui->comboBoxStopbits;
    comboBoxParity           = ui->comboBoxParity;
    checkBoxCustomBaudrate   = ui->checkBoxCustomBaudrate;

    refresh();
    ui->comboBoxSerialports->installEventFilter(this);
}

SAKSerialPortDeviceController::~SAKSerialPortDeviceController()
{
    delete ui;
}


void SAKSerialPortDeviceController::refresh()
{
    SAKGlobal::initComComboBox(comboBoxSerialports);
    SAKGlobal::initBaudRateComboBox(comboBoxBaudrate);
    SAKGlobal::initDataBitsComboBox(comboBoxDatabits);
    SAKGlobal::initStopBitsComboBox(comboBoxStopbits);
    SAKGlobal::initParityComboBox(comboBoxParity);
}

void SAKSerialPortDeviceController::setUiEnable(bool enable)
{
    comboBoxSerialports->setEnabled(enable);
    comboBoxBaudrate->setEnabled(enable);
    comboBoxDatabits->setEnabled(enable);
    comboBoxStopbits->setEnabled(enable);
    comboBoxParity->setEnabled(enable);
    checkBoxCustomBaudrate->setEnabled(enable);
}

enum QSerialPort::DataBits SAKSerialPortDeviceController::dataBits()
{
    return static_cast<QSerialPort::DataBits>(comboBoxDatabits->currentData().toInt());
}

enum QSerialPort::StopBits SAKSerialPortDeviceController::stopBits()
{
    return static_cast<QSerialPort::StopBits>(comboBoxStopbits->currentData().toInt());
}

enum QSerialPort::Parity SAKSerialPortDeviceController::parity()
{
    return static_cast<QSerialPort::Parity>(comboBoxParity->currentData().toInt());
}

QString SAKSerialPortDeviceController::name()
{
    QString portName = comboBoxSerialports->currentText();
    portName = portName.split(' ').first();
    return  portName;
}

qint32 SAKSerialPortDeviceController::baudRate()
{
    qint32 rate = 9600;
    bool ok = false;
    if (comboBoxBaudrate->currentText().isEmpty()){
        return 9600;
    }

    rate = comboBoxBaudrate->currentText().toInt(&ok);
    if (!ok){
        rate = 9600;
    }

    return rate;
}

void SAKSerialPortDeviceController::refreshCom()
{
}

void SAKSerialPortDeviceController::on_checkBoxCustomBaudrate_clicked()
{
    if (checkBoxCustomBaudrate->isChecked()){
        comboBoxBaudrate->setEditable(true);
        comboBoxBaudrate->lineEdit()->selectAll();
        comboBoxBaudrate->lineEdit()->setFocus();
    }else{
        comboBoxBaudrate->setEditable(false);
    }
}

void SAKSerialPortDeviceController::on_comboBoxSerialports_activated(int index)
{
}

void SAKSerialPortDeviceController::on_comboBoxSerialports_activated(const QString &arg1)
{

}

void SAKSerialPortDeviceController::updatePort()
{
    QStringList newPortStringList;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
    {
        newPortStringList += info.portName();
    }

}


bool SAKSerialPortDeviceController::eventFilter(QObject *watched, QEvent *event)
{
    static int8_t refreshTime = 0;

   if (watched == ui->comboBoxSerialports)
   {
       if (event->type() == QEvent::MouseButtonPress)
       {
            SAKGlobal::initComComboBox(ui->comboBoxSerialports);

            qDebug() << refreshTime++;
       }
   }
   return QWidget::eventFilter(watched, event);
}
