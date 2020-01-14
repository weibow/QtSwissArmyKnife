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
#ifndef SAKDEBUGPAGEOUTPUTMANAGER_HH
#define SAKDEBUGPAGEOUTPUTMANAGER_HH

#include "SAKDebugPage.hh"

#include <QTimer>
#include <QObject>
#include <QVector>


class SAKOutputDataFactory;
class SAKSaveOutputDataSettings;
class SAKDebugPageOutputManager:public QObject
{
    Q_OBJECT
public:
    SAKDebugPageOutputManager(SAKDebugPage *debugPage, QObject *parent = nullptr);
    ~SAKDebugPageOutputManager();

    /**
     * @brief 输出参数
     */
    struct OutputParameters {
        bool showDate;      // 是否显示日期
        bool showTime;      // 是否显示时间
        bool showMS;        // 是否显示毫秒
        bool isReceivedData;// 是否为接收到的数据
        int  textModel;     // 输出数据格式SAKGlobal::SAKTextFormat
        int protocol;       // current Protocol
    };
public:
       ///Clears all stored data.
    void clear(void);

    ///Reinserts the data into the consoles.
    void reInsertDataInConsole(void);

private:
    ///The data buffer for the ascii console.
    QString m_consoleDataBufferAscii;

    ///The data buffer for the hex console.
    QString m_consoleDataBufferHex;

    ///The data buffer for the decimal console.
    QString m_consoleDataBufferDec;

    //Add the new data type
    ///The unprocessed console data.
    QVector<StoredData> m_unprocessedConsoleData;

    ///The unprocessed log data.
    QVector<StoredData> m_unprocessedLogData;

    ///Bytes in m_unprocessedLogData;
    quint32 m_bytesInUnprocessedConsoleData;

    ///The stored console data.
    QVector<StoredData> m_storedConsoleData;

    ///Bytes in m_storedConsoleData;
    quint32 m_bytesInStoredConsoleData;

private:
        ///Processes the data in m_storedData (creates the log and the console strings).
    ///Note: m_storedData is cleared in this function.
   void processDataInStoredData();
   ///Appends data to the console strings (m_consoleDataBufferAscii, m_consoleDataBufferHex;
   ///m_consoleDataBufferDec)
   void appendDataToConsoleStrings(QByteArray& data, bool isSend, bool isUserMessage,
                                   bool isTimeStamp, bool isFromCan, bool isFromI2cMaster, bool isNewLine);

    //System origial data
    SAKDebugPage *debugPage;
    SAKOutputDataFactory *dataFactory;
    SAKSaveOutputDataSettings *outputSettings;

    QTimer updateRxFlagTimer;
    void updateRxFlag();
    qint8 rxFlagCount;

    QTimer updateTxFlagTimer;
    void updateTxFlag();
    qint8 txFlagCount;

    void setLineWrapMode();
    void saveOutputTextToFile();
    void saveOutputDataSettings();
    void saveOutputDataToFile();

    /*
     * ui指针
     */
    QTextBrowser *messageTextBrowser;
    QLabel       *rxLabel;
    QLabel       *txLabel;
    QComboBox    *outputTextFormatComboBox;
    QComboBox 	 *protocalComboBox;
    QCheckBox    *showDateCheckBox;
    QCheckBox    *autoWrapCheckBox;
    QCheckBox    *showTimeCheckBox;
    QCheckBox    *showMsCheckBox;
    QCheckBox    *showRxDataCheckBox;
    QCheckBox    *showTxDataCheckBox;
    QCheckBox    *saveOutputFileToFilecheckBox;
    QPushButton  *outputFilePathPushButton;
    QPushButton  *clearOutputPushButton;
    QPushButton  *saveOutputPushButton;
    QTextBrowser *outputTextBroswer;
    QTextBrowser *weightTextBroswer;


private:
    void bytesRead(QByteArray data);
    void bytesWritten(QByteArray data);
    void outputData(QString data);
    void outWeightData(QByteArray data, SAKDebugPageOutputManager::OutputParameters parameters);
    OutputParameters outputDataParameters(bool isReceivedData);
signals:
    void cookData(QByteArray rawData, OutputParameters parameters);
    void weightData(QByteArray weightdata);

public slots:
    //Reinserts the data into the mixed consoles.




};
Q_DECLARE_METATYPE(SAKDebugPageOutputManager::OutputParameters);

#endif
