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
#include "SAKGlobal.hh"
#include "SAKOutputDataFactory.hh"
#include "SAKDebugPageOutputManager.hh"
#include "SAKSaveOutputDataSettings.hh"

#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QFileDialog>
#include <QDebug>
#include <QtEndian>

SAKDebugPageOutputManager::SAKDebugPageOutputManager(SAKDebugPage *debugPage, QObject *parent)
    :QObject (parent)
    ,debugPage (debugPage)
    ,rxFlagCount (4)
    ,txFlagCount (0)
{
    // 注册数据类型（自定义数据类型作为信号参数时，要进行注册，该函数要与宏 Q_DECLARE_METATYPE 配套使用）
    qRegisterMetaType<OutputParameters>("OutputParameters");

    rxLabel                         = debugPage->rxLabel;
    txLabel                         = debugPage->txLabel;
    outputTextFormatComboBox        = debugPage->outputTextFormatComboBox;
    protocalComboBox				= debugPage->protocalComboBox;

    showDateCheckBox                = debugPage->showDateCheckBox;
    autoWrapCheckBox                = debugPage->autoWrapCheckBox;
    showTimeCheckBox                = debugPage->showTimeCheckBox;
    showMsCheckBox                  = debugPage->showMsCheckBox;
    showRxDataCheckBox              = debugPage->showRxDataCheckBox;
    showTxDataCheckBox              = debugPage->showTxDataCheckBox;
    saveOutputFileToFilecheckBox    = debugPage->saveOutputFileToFilecheckBox;
    outputFilePathPushButton        = debugPage->outputFilePathPushButton;
    clearOutputPushButton           = debugPage->clearOutputPushButton;
    saveOutputPushButton            = debugPage->saveOutputPushButton;
    outputTextBroswer               = debugPage->outputTextBroswer;
    weightTextBroswer 				= debugPage->outputWeightBroswer;

    connect(saveOutputFileToFilecheckBox,   &QCheckBox::clicked, this, &SAKDebugPageOutputManager::saveOutputDataToFile);
    connect(autoWrapCheckBox,               &QCheckBox::clicked, this, &SAKDebugPageOutputManager::setLineWrapMode);
    connect(saveOutputPushButton,           &QCheckBox::clicked, this, &SAKDebugPageOutputManager::saveOutputTextToFile);
    connect(outputFilePathPushButton,       &QCheckBox::clicked, this, &SAKDebugPageOutputManager::saveOutputDataSettings);

    // 初始化数据格式预选框
    SAKGlobal::initOutputTextFormatComboBox(outputTextFormatComboBox);
    SAKGlobal::initProtocolComboBox(protocalComboBox);//)

    /*
     * 处理已接收或者是已发送的数据
     */
    connect(debugPage, &SAKDebugPage::bytesRead, this, &SAKDebugPageOutputManager::bytesRead);
    connect(debugPage, &SAKDebugPage::bytesWritten, this, &SAKDebugPageOutputManager::bytesWritten);

    /*
     * 数据先发送到 dataFactory 中进行处理，处理完毕后再输出至界面
     */
    dataFactory = new SAKOutputDataFactory;
    connect(this, &SAKDebugPageOutputManager::cookData, dataFactory, &SAKOutputDataFactory::cookData);
    connect(dataFactory, &SAKOutputDataFactory::dataCooked, this, &SAKDebugPageOutputManager::outputData);
    connect(dataFactory, &SAKOutputDataFactory::weightCooked, this, &SAKDebugPageOutputManager::outWeightData);
    dataFactory->start();

    outputSettings = new SAKSaveOutputDataSettings;

    updateRxFlagTimer.setInterval(20);
    updateTxFlagTimer.setInterval(20);
    connect(&updateRxFlagTimer, &QTimer::timeout, this, &SAKDebugPageOutputManager::updateRxFlag);
    connect(&updateTxFlagTimer, &QTimer::timeout, this, &SAKDebugPageOutputManager::updateTxFlag);

    outputTextBroswer->document()->setMaximumBlockCount(10);
    weightTextBroswer->document()->setMaximumBlockCount(1);

}

SAKDebugPageOutputManager::~SAKDebugPageOutputManager()
{
    dataFactory->terminate();
    delete dataFactory;
    delete outputSettings;
}

void SAKDebugPageOutputManager::clear()
{
    m_consoleDataBufferAscii.clear();
    m_consoleDataBufferDec.clear();;
    m_consoleDataBufferHex.clear();
    m_bytesInStoredConsoleData = 0;
    m_bytesInUnprocessedConsoleData = 0;
    m_storedConsoleData.clear();
    m_unprocessedConsoleData.clear();
}

void SAKDebugPageOutputManager::reInsertDataInConsole()
{

}

/* CRC-ITU 1021 表*/
const uint16_t fcstab[] =
{
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0,
};

//函数  计算指定缓冲区的数据校验和
//      校验和计算方法使用查表的方式完成
//输入  fcs 校验和初始值
//      buf 数据地址
//      len 数据长度
//输出  无
//返回  校验值
uint16_t Drv_CRC16_Calculate(uint8_t *buf, uint8_t len)
{
    uint16_t fcs = 0;
    while (len--)
    {
        fcs = (fcs << 8) ^ fcstab[(fcs >> 8) ^ *buf];
        buf++;
    }
    return (fcs);
}

/*
 * Add the CRC check
 */
//quint32 SAKDebugPageOutputManager::crcCalculate(QByteArray data, int model)
//{
//    uint8_t crc8;
//    uint16_t crc16;
//    uint32_t crc32;
//    quint32 crc = 0;
//    uint8_t bitsWidth = 8;

//    switch (bitsWidth) {
//    case 8:
//        crc8 = SAKCRCInterface::crcCalculate<uint8_t>(reinterpret_cast<uint8_t*>(data.data()), static_cast<quint64>(data.length()), static_cast<SAKCRCInterface::CRCModel>(model));
//        crc = crc8;
//        break;
//    case 16:
//        crc16 = SAKCRCInterface::crcCalculate<uint16_t>(reinterpret_cast<uint8_t*>(data.data()), static_cast<quint64>(data.length()), static_cast<SAKCRCInterface::CRCModel>(model));
//        crc = crc16;
//        break;
//    case 32:
//        crc32 = SAKCRCInterface::crcCalculate<uint32_t>(reinterpret_cast<uint8_t*>(data.data()), static_cast<quint64>(data.length()), static_cast<SAKCRCInterface::CRCModel>(model));
//        crc = crc32;
//        break;
//    default:
//        break;
//    }

//    return crc;
//}

uint16_t SAKDebugPageOutputManager::Drv_CRC16_Calculate(uint8_t *buf, uint8_t len)
{
    uint16_t fcs = 0;
    while (len--)
    {
        fcs = (fcs << 8) ^ fcstab[(fcs >> 8) ^ *buf];
        buf++;
    }
    return (fcs);
}

/*
 *
 */
uint8_t SAKDebugPageOutputManager::add_sum(uint8_t *_pBuf, uint16_t _usLen)
{
    uint8_t checksum;

    checksum  = 0;
    while (_usLen--) {
        checksum += *_pBuf++;
    }
    return checksum;
}

/**
 * Processes the data in m_unprocessedConsoleData (creates the log and the console strings).
 *
 * Note: m_unprocessedConsoleData is cleared in this function.
 */
void SAKDebugPageOutputManager::processDataInStoredData()
{
    for (auto el : m_unprocessedConsoleData)
    {
        if (el.type == STORED_DATA_CLEAR_ALL_STANDARD_CONSOLES) {
            m_consoleDataBufferAscii.clear();
            m_consoleDataBufferDec.clear();
            m_consoleDataBufferHex.clear();

            m_storedConsoleData.clear();
            m_bytesInStoredConsoleData = 0;

        } else {
            bool isFromAddMessageDialog = (el.type == STORED_DATA_TYPE_USER_MESSAGE) ? true : false;
            bool isTimeStamp = (el.type == STORED_DATA_TYPE_TIMESTAMP) ? true : false;
            bool isNewLine = (el.type == STORED_DATA_TYPE_NEW_LINE) ? true : false;

            m_storedConsoleData.push_back(el);
            m_bytesInStoredConsoleData += el.data.size();
            appendDataToConsoleStrings(el.data, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                       el.isFromCan, el.isFromI2cMaster, isNewLine);
        }
    }	//for(auto el : m_umprocessedConsoleData)

    m_unprocessedConsoleData.clear();
    m_bytesInUnprocessedConsoleData = 0;
    m_unprocessedLogData.clear();
}

/*
 *
 */
void SAKDebugPageOutputManager::appendDataToConsoleStrings(QByteArray &data, bool isSend, bool isUserMessage, bool isTimeStamp, bool isFromCan, bool isFromI2cMaster, bool isNewLine)
{
}

void SAKDebugPageOutputManager::updateRxFlag()
{
    updateRxFlagTimer.stop();
    rxLabel->setText(QString("C%1").arg(QString(""), rxFlagCount, '<'));

    rxFlagCount -= 1;
    if (rxFlagCount == -1){
        rxFlagCount = 8;
    }
}

void SAKDebugPageOutputManager::updateTxFlag()
{
    updateTxFlagTimer.stop();
    txLabel->setText(QString("C%1").arg(QString(""), txFlagCount, '>'));

    txFlagCount += 1;
    if (txFlagCount == 8){
        txFlagCount = 0;
    }
}

void SAKDebugPageOutputManager::setLineWrapMode()
{
    if (autoWrapCheckBox->isChecked()){
        outputTextBroswer->setLineWrapMode(QTextEdit::WidgetWidth);
    }else{
        outputTextBroswer->setLineWrapMode(QTextEdit::NoWrap);
    }
}

void SAKDebugPageOutputManager::saveOutputTextToFile()
{
    QString outFileName = QFileDialog::getSaveFileName(nullptr,
                                                       tr("保存文件"),
                                                       QString("./%1.txt")
                                                       .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss")),
                                                       tr("文本 (*.txt)"));
    if (outFileName.isEmpty()){
        return;
    }

    QFile outFile(outFileName);
    if(outFile.open(QIODevice::WriteOnly|QIODevice::Text)){
        QTextStream outStream(&outFile);
        outStream << outputTextBroswer->toPlainText();
        outFile.flush();
        outFile.close();
    } else {
        debugPage->outputMessage(QString("Can not open file (%1) to save output data:")
                                 .arg(outFile.fileName()) + outFile.errorString(), false);
    }
}

void SAKDebugPageOutputManager::saveOutputDataSettings()
{
    outputSettings->show();
}

void SAKDebugPageOutputManager::saveOutputDataToFile()
{
    if (saveOutputFileToFilecheckBox->isChecked()){
        connect(debugPage, &SAKDebugPage::bytesRead, outputSettings, &SAKSaveOutputDataSettings::inputData);
    }else{
        disconnect(debugPage, &SAKDebugPage::bytesRead, outputSettings, &SAKSaveOutputDataSettings::inputData);
    }
}

void SAKDebugPageOutputManager::bytesRead(QByteArray data)
{
    if (!updateRxFlagTimer.isActive()){
        updateRxFlagTimer.start();
    }

    if (!showRxDataCheckBox->isChecked()){
        return;
    }

    OutputParameters parameters = outputDataParameters(true);
    emit cookData(data, parameters);
}

void SAKDebugPageOutputManager::bytesWritten(QByteArray data)
{
    if (!updateTxFlagTimer.isActive()){
        updateTxFlagTimer.start();
    }

    if (!showTxDataCheckBox->isChecked()){
        return;
    }

    OutputParameters parameters = outputDataParameters(false);
    emit cookData(data, parameters);
}

void SAKDebugPageOutputManager::outputData(QString data)
{
    outputTextBroswer->append(data);
}

/*
 * OutWeightData
 */
void SAKDebugPageOutputManager::outWeightData(QByteArray data, SAKDebugPageOutputManager::OutputParameters parameters)
{
    QString str;
    QString weight = QString::fromLocal8Bit(data);
    QString tmpStr;
    float tmpFloat;
    bool ok;
    static QByteArray resData;
    int i, j;
    int len;

    PackageHead *packhead = NULL;// (PackageHead*)(resData.data() + );

    switch (parameters.protocol)
    {
    case 0:			//yaoHua A12E
        str.append("<span style='white-space:pre;'>");
        str.append("<font color=green size=72>");
        if (weight.startsWith("wn")) {
            str.append("净重:");
        } else if (weight.startsWith("ww")) {
            str.append("毛重:");
        } else if (weight.startsWith("wt")) {
            str.append("皮重:");
        }
        str.append("</font>");
        str.append("<font color=red size=72>" );
        tmpFloat = weight.mid(2, 7).toFloat(&ok);
        qDebug() << weight.mid(2, 7);
        qDebug() << (6 - weight.mid(2,7).indexOf("."));
        switch (6 -  weight.mid(2, 7).indexOf(".")) {
        case 0:
        tmpStr.sprintf("%8f", tmpFloat);
        break;
         case 1:
        tmpStr.sprintf("%8.1f", tmpFloat);
        break;
         case 2:
        tmpStr.sprintf("%8.2f", tmpFloat);
        break;
         case 3:
        tmpStr.sprintf("%8.3f", tmpFloat);
        break;
         case 4:
        tmpStr.sprintf("%8.4f", tmpFloat);
        break;
         default:
        tmpStr.sprintf("%7.0f", tmpFloat);
        break;
        }
        str.append(tmpStr);
        str.append("</font>");
        str.append("<font color=blue size=30>");
        str.append(weight.right(2));
        str.append("</font></span>");
        weightStyleStr = str;
        weightTextBroswer->document()->setMaximumBlockCount(1);
        break;
     case 1:		//CAS protocol
        break;
     case 2:		//Label protocolfromLatin1
        qDebug() << "printf first";
        QString str2;
        for (j = 0; j < resData.size(); ++j)
        {
           str2.append(QString("%1 ").arg(QString::number(static_cast<uint8_t>(resData.at(j)), 16), 2, '0'));
        }
        qDebug() << str2;
        resData.append(data);
        str2.clear();
        for (j = 0; j < resData.size(); ++j)
        {
           str2.append(QString("%1 ").arg(QString::number(static_cast<uint8_t>(resData.at(j)), 16), 2, '0'));
        }
        qDebug() << str2;
        qDebug() << "printf end";
        int ret = 0;

        if (resData.size() >= 17) {
            for (i = 0; i < (resData.size() - 16); ++i) {
                if (ret == 1)
                    break;
                if (resData.at(i) == 0x03 && resData.at(i + 16) == 0x04) {
                    packhead = NULL;
                    if (add_sum(reinterpret_cast<uint8_t*>(resData.data()) + i + 1, 14) == static_cast<uint8_t>(resData.at(i + 15))) {
                        packhead = (PackageHead*)(resData.data() + i);
                        int cmd = qFromBigEndian<quint32>(packhead->cmd);
                        len = qFromBigEndian(packhead->len);
//                        qDebug() << i << QString("%1").arg(cmd,8 , 16) << packhead->STX <<  "a0" << QString("%1").arg(len, 8, 16);
                        if (len > 512) { 				//The Maxmum data Size
                            continue;
                        }
                        if (resData.size() < (i + len + 17)) {
                            return;
                        }
                        for (j = i; j < resData.size(); j++)
                        {
                           str.append(QString("%1 ").arg(QString::number(static_cast<uint8_t>(resData.at(j)), 16), 2, '0'));
                        }

                        qDebug() << "A1" << str;
                        qDebug() << "B1" << QString("%1").arg(Drv_CRC16_Calculate(reinterpret_cast<uint8_t*>(resData.data() + i + 17), len-2), 4, 16);
                        if (Drv_CRC16_Calculate(reinterpret_cast<uint8_t*>(resData.data() + i + 17), len) != 0)
                            continue;
                        LabelProtocol_process(reinterpret_cast<uint8_t*>(resData.data() + i + 17), len - 2, cmd);
                        weightTextBroswer->document()->setMaximumBlockCount(2);
                        resData = resData.right(resData.size() - len - i - 17);
                        QString str2;
                        for (j = 0; j < resData.size(); ++j)
                        {
                           str2.append(QString("%1 ").arg(QString::number(static_cast<uint8_t>(resData.at(j)), 16), 2, '0'));
                        }
                        qDebug() << "A2" << str2;
                        ret = 1;
                        break;
                    }
                }
            }
        } else {
            return ;
        }
        break;
    }

    weightTextBroswer->append(weightStyleStr);
    weightStyleStr.clear();
}

/*
 *
 */
SAKDebugPageOutputManager::OutputParameters SAKDebugPageOutputManager::outputDataParameters(bool isReceivedData)
{
    OutputParameters parameters;
    parameters.showDate = showDateCheckBox->isChecked();
    parameters.showTime = showTimeCheckBox->isChecked();
    parameters.showMS   = showMsCheckBox->isChecked();
    parameters.isReceivedData = isReceivedData;
    parameters.textModel= outputTextFormatComboBox->currentData().toInt();
    parameters.protocol = protocalComboBox->currentIndex();
    return parameters;
}


/*
 *
 */
void SAKDebugPageOutputManager::LabelProtocol_process(uint8_t *data, int32_t len, uint32_t cmd)
{
    QString tmpStr;
    float tmpFloat;

    switch(cmd){
    case CMD_AD_READ:
        WeightT *tmpWeight; 			// = (WeightT*) data;
        tmpWeight = (WeightT*) data;
        qDebug() << tmpWeight->Dot << qFromBigEndian(tmpWeight->Flag) << qFromBigEndian(tmpWeight->Net_Wei) << qFromBigEndian(tmpWeight->Tare_Wei) << qFromBigEndian(tmpWeight->Voltage);
        qDebug() << QString("%1").arg(qFromBigEndian(tmpWeight->Net_Wei));

        /* Add NetWeight*/
        tmpStr =QString("%1").arg(qFromBigEndian(tmpWeight->Net_Wei), 10, 10, QLatin1Char('0'));
        if (tmpWeight->Dot > 0)
        {
            tmpStr.insert(tmpStr.size() - tmpWeight->Dot, ".");// )
        }
        qDebug() << tmpStr;
        tmpFloat = tmpStr.toFloat();
        switch (tmpWeight->Dot) {
         case 0:
            tmpStr.sprintf("%8f", tmpFloat);
            break;
         case 1:
            tmpStr.sprintf("%8.1f", tmpFloat);
            break;
         case 2:
            tmpStr.sprintf("%8.2f", tmpFloat);
            break;
         case 3:
            tmpStr.sprintf("%8.3f", tmpFloat);
            break;
         case 4:
            tmpStr.sprintf("%8.4f", tmpFloat);
            break;
         default:
            tmpStr.sprintf("%7.0f", tmpFloat);
            break;
        }
        qDebug() << tmpStr;
        weightStyleStr.append("<span style='white-space:pre;'>");
        weightStyleStr.append("<font color=green size=72>");
        weightStyleStr.append("净重:");
        weightStyleStr.append("</font>");
        weightStyleStr.append("<font color=red size=72>" );
        weightStyleStr.append(tmpStr);
        weightStyleStr.append("</font>");
        weightStyleStr.append("<font color=blue size=30>");

        if (tmpWeight->Unit == 0) {
            weightStyleStr.append("kg");
        } else if (tmpWeight->Unit == 1) {
            weightStyleStr.append("catty");
        } else if (tmpWeight->Unit == 2) {
            weightStyleStr.append("lb");
        }
        weightStyleStr.append("\r\n");

        /*Add the Tare weight*/
        tmpStr =QString("%1").arg(qFromBigEndian(tmpWeight->Tare_Wei), 10, 10, QLatin1Char('0'));
        if (tmpWeight->Dot > 0)
        {
            tmpStr.insert(tmpStr.size() - tmpWeight->Dot, ".");
        }
        qDebug() << tmpStr;
        tmpFloat = tmpStr.toFloat();
        switch (tmpWeight->Dot) {
         case 0:
            tmpStr.sprintf("%8f", tmpFloat);
            break;
         case 1:
            tmpStr.sprintf("%8.1f", tmpFloat);
            break;
         case 2:
            tmpStr.sprintf("%8.2f", tmpFloat);
            break;
         case 3:
            tmpStr.sprintf("%8.3f", tmpFloat);
            break;
         case 4:
            tmpStr.sprintf("%8.4f", tmpFloat);
            break;
         default:
            tmpStr.sprintf("%7.0f", tmpFloat);
            break;
        }
        qDebug() << tmpStr;

        weightStyleStr.append("<font color=green size=72>");
        weightStyleStr.append("皮重:");
        weightStyleStr.append("</font>");
        weightStyleStr.append("<font color=red size=72>" );
        weightStyleStr.append(tmpStr);
        weightStyleStr.append("</font>");
//        weightStyleStr.append("<font color=blue size=30>");
        weightStyleStr.append("<font color=blue size=30>");

        if (tmpWeight->Unit == 0) {
            weightStyleStr.append("kg");
        } else if (tmpWeight->Unit == 1) {
            weightStyleStr.append("catty");
        } else if (tmpWeight->Unit == 2) {
            weightStyleStr.append("lb");
        }
//        weightStyleStr.append("\r\n");
        weightStyleStr.append("</font></span>");

        break;
    case CMD_PARAMETER_SET:
        break;
    }
}

