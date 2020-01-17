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
//    #pragma pack(push, 1)
    #pragma pack (1)
   typedef struct {
        uint8_t STX;		//1
        uint32_t PackageNO; //5
        uint32_t cmd;		//9
        uint32_t res;		//13
        uint16_t len;		//15
        uint8_t sum;		//16
        uint8_t ETX;		//17
    } PackageHead;
//    #pragma pack(pop)
    #pragma pack ()

#pragma pack(1)
    typedef struct
    {
        int32_t Net_Wei; // 有符号净重
        int32_t Tare_Wei; // 有符号皮重
        uint8_t Unit; // 单位
        uint8_t Dot; // 小数位数
        uint16_t Flag; // 测量结果标志
        uint16_t Voltage;
    } WeightT;
#pragma pack()


    enum WeightCmd{
        CMD_TEST_COM 	  = 0xA8000,
        CMD_PEEL_SET	  = 0xC0009,
        CMD_ZERO_SET	  = 0xC0007,
        CMD_AD_READ		  = 0xC0008,

        CMD_ZERO_CALI 	  = 0xC000A,
        CMD_FULL_CALI 	  = 0xC000B,
        CMD_ACCELE_SET 	  = 0xC000C,
        CMD_DOT_SET 	  = 0xC000D,
        CMD_REPAIR_SET 	  = 0xC000E,
        CMD_FULL_SET	  = 0xC000F,
        CMD_DIV_SET		  = 0xC0010,
        CMD_UNIT_SET	  = 0xC0011,
        CMD_PEEL_MODE	  = 0xC0015,
        CMD_TEMPERATURE   = 0xC0016,
        CMD_WEIGHT_MODE   = 0xC0017,
        CMD_MIN_WEIGHT    = 0xC0018,
        CMD_PARAMETER_SET = 0xC0012,
        CMD_FACTORY_MODE  = 0xC0014,
        CMD_READ_VER	  = 0xC0023,

    };


public:
       ///Clears all stored data.
    void clear(void);

    ///Reinserts the data into the consoles.
    void reInsertDataInConsole(void);
//    quint32 crcCalculate(QByteArray data, int model);
    uint16_t Drv_CRC16_Calculate(uint8_t *buf, uint8_t len);
    /*
     * Add sum caculate
     */
    uint8_t add_sum(uint8_t *_pBuf, uint16_t _usLen);

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

    QString weightStyleStr;

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
    void LabelProtocol_process(uint8_t* data,int32_t len, uint32_t cmd);
    void CasProtocol_process(uint8_t* data, int32_t len);
signals:
    void cookData(QByteArray rawData, OutputParameters parameters);
    void weightData(QByteArray weightdata);

public slots:
    //Reinserts the data into the mixed consoles.

};
Q_DECLARE_METATYPE(SAKDebugPageOutputManager::OutputParameters);

#endif
