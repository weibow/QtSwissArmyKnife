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
#ifndef SAKUDPDEVICECONTROLLER_HH
#define SAKUDPDEVICECONTROLLER_HH

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>

namespace Ui {
    class SAKUdpDeviceController;
}

class SAKUdpDeviceController:public QWidget
{
    Q_OBJECT
public:
    SAKUdpDeviceController(QWidget *parent = Q_NULLPTR);
    ~SAKUdpDeviceController();

    QString localHost();
    quint16 localPort();
    QString targetHost();
    quint16 targetPort();
    bool enableCustomLocalSetting();

    void refresh();
    void setUiEnable(bool enable);
private:
    Ui::SAKUdpDeviceController *ui;

    QComboBox *localhostComboBox;
    QLineEdit *localPortlineEdit;
    QCheckBox *enableLocalSettingCheckBox;
    QLineEdit *targetHostLineEdit;
    QLineEdit *targetPortLineEdit;
    QCheckBox *broadcastCheckBox;
    QLineEdit *broadcastPortLineEdit;
    QCheckBox *multicastCheckBox;
    QLineEdit *multicastAddressLineEdit;
    QLineEdit *multicastPortLineEdit;
};

#endif
