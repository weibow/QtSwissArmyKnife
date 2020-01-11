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
#include "SAKSettings.hh"
#include "SAKApplication.hh"
#include "SAKMainWindow.hh"
#include "SAKGlobal.hh"
#include "SAKSerialPortTransmissionItemWidget.hh"
#include "SAKSerialPortDeviceController.hh"

#include <QTimer>
#include <QSettings>
#include <QTextCursor>
#include <QTranslator>
#include <QDebug>

SAKApplication::SAKApplication(int argc, char **argv):
    QApplication(argc, argv)
{
    installLanguage();
    setApplicationVersion(QString("2.2.0"));

    /// 注册表选项
    setOrganizationName(QString("Qter"));
    setOrganizationDomain(QString("IT"));
    setApplicationName(QString("QtSwissArmyKnife"));

    mpMainWindow = new SAKMainWindow;
    mpMainWindow->show();

    QTimer::singleShot(1, [=](){
        if (SAKSettings::instance()->enableAutoCheckForUpdate()){
            emit this->checkForUpdate();
        }
        qDebug() <<"I";
        qDebug() <<"see";
    });
}


SAKApplication::~SAKApplication()
{

}

/*
 *
 */
void SAKApplication::installLanguage()
{
    QString path = qApp->applicationDirPath();
    QString fileName = path+"/system.ini";
    QSettings settings(fileName, QSettings::IniFormat);

    static QTranslator* translator;
    QString language = SAKSettings::instance()->value(_settingStringLanguage).toString();
    QString qmName;
    if (language.isEmpty()) {
        if (QLocale().country() == QLocale::China){
            qmName = QString("zh_CN");
        } else {
            qmName = QString("en");
        }
    } else {
        qmName = language.split('-').first();
    }

    if (translator != NULL) {
        qApp->removeTranslator(translator);
        delete translator;
        translator = NULL;
    }
    translator = new QTranslator;
    translator->load(QString(":/translations/sak/SAK_%1.qm").arg(qmName));
    qApp->installTranslator(translator);

    if (sender()) {
        QAction *action = reinterpret_cast<QAction*>(sender());
        action->setChecked(true);
        QString title = action->data().toString();
    }
    qDebug() << "hello" ;
}
