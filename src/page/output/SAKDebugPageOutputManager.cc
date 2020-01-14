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
   QByteArray * dataArray = &data;
   QByteArray tmpArray;

   QString additionalInformation;
   if (data.isEmpty())
       return;
   if (isNewLine)
   {

   }
   else
   {
       if (isUserMessage || isTimeStamp)
       {

       }
       else
       {

       }
   }
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
    }else{
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
 *
 */
void SAKDebugPageOutputManager::outWeightData(QByteArray data, SAKDebugPageOutputManager::OutputParameters parameters)
{
    QString str;
    QString weight = QString::fromLocal8Bit(data);
    QString tmpStr;
    float tmpFloat;
    bool ok;
    static QByteArray resData;


    switch (parameters.protocol)
    {
    case 0:			//yaoHua A12E
        str.append("<font color=green size=72>");
        if (weight.startsWith("wn")) {
        str.append("净重:");
        } else if (weight.startsWith("ww")) {
        str.append("毛重:");
        } else if (weight.startsWith("wt")) {
        str.append("皮重:");
        }
        str.append("</font>");
        str.append("<font color=red size=72><span style='white-space:pre;'>" );
        tmpFloat = weight.mid(2, 7).toFloat(&ok);
        switch (6 -  weight.mid(2, 7).indexOf(".", -1)) {
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
        str.append("</font> </span>");
        str.append("<font color=blue size=30>");
        str.append(weight.right(2));
        str.append("\r\nseeyouagain");
     case 1:		//CAS protocol
        break;
     case 2:		//Label protocolfromLatin1
        for (int i = 0; i < data.length(); i++){
            str.append(QString("%1 ").arg(QString::number(static_cast<uint8_t>(data.at(i)), 16), 2, '0'));
        }
        resData.append(data);
        break;
    }

    weightTextBroswer->append(str);
}

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
