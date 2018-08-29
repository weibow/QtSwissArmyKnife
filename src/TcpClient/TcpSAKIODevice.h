﻿/*******************************************************************************
* The file is encoding with utf-8 (with BOM)
*
* I write the comment with English, it's not because that I'm good at English,
* but for "installing B".
*
* Copyright (C) 2018-2018 wuhai persionnal. All rights reserved.
*******************************************************************************/
#ifndef TCPSAKIODEVICE_H
#define TCPSAKIODEVICE_H

#include <QTcpSocket>
#include <QThread>
#include <QDebug>
#include <QHostAddress>

#include "SAKIODevice.h"

class TcpSAKIODevice : public SAKIODevice
{
    Q_OBJECT
public:
    TcpSAKIODevice(SAKDeviceType deviceType = SAKDeviceTcp, QObject *parent = Q_NULLPTR);
    ~TcpSAKIODevice();

    virtual bool isOpen();
    virtual QString errorString(){return mpTcpSocket->errorString();}
public slots:
    virtual void writeBytes(QByteArray data);
    virtual void open(QString hostAddress, QString hostPort, QString peerAddress, QString peerPort);
    virtual void close();

    virtual SAKDeviceType deviceType(){return SAKDeviceTcp;}
protected:
    void run();
private:
    QTcpSocket *mpTcpSocket = NULL;
private slots:
    void readBytes();
    void afterDisConnect();
    void resetTheDevice();
};

#endif