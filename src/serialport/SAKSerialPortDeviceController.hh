﻿/*
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
#ifndef SAKSERIALPORTDEVICECONTROLLER_HH
#define SAKSERIALPORTDEVICECONTROLLER_HH

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSerialPort>

namespace Ui {
    class SAKSerialPortDeviceController;
}

class SAKSerialPortDeviceController:public QWidget
{
    Q_OBJECT
public:
    SAKSerialPortDeviceController(QWidget *parent = Q_NULLPTR);
    ~SAKSerialPortDeviceController();

    enum QSerialPort::DataBits  dataBits();
    enum QSerialPort::StopBits  stopBits();
    enum QSerialPort::Parity    parity();

    QString                     name();
    qint32                      baudRate();

    void refreshCom();


    void refresh();
    void setUiEnable(bool enable);
private:
    Ui::SAKSerialPortDeviceController *ui;

    QComboBox *comboBoxSerialports      = nullptr;
    QComboBox *comboBoxBaudrate         = nullptr;
    QComboBox *comboBoxDatabits         = nullptr;
    QComboBox *comboBoxStopbits         = nullptr;
    QComboBox *comboBoxParity           = nullptr;
    QCheckBox *checkBoxCustomBaudrate   = nullptr;
private slots:
    void on_checkBoxCustomBaudrate_clicked();
    void on_comboBoxSerialports_activated(int index);
    void on_comboBoxSerialports_activated(const QString &arg1);
public slots:
    void updatePort();						//refresh the port;
public slots:
    bool eventFilter(QObject *watched, QEvent *event);
};

#endif
