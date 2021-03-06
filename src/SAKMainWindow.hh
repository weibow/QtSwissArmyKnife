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
#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMenu>
#include <QLabel>
#include <QAction>
#include <QMetaEnum>
#include <QTabWidget>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QTranslator>

class SAKUpdateManager;
class SAKMoreInformation;

namespace Ui {
class SAKMainWindow;
}

class SAKMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SAKMainWindow(QWidget *parent = nullptr);
    ~SAKMainWindow();

protected:
      bool eventFilter(QObject *obj, QEvent *event);

private:
    QTabWidget* mpTabWidget             = nullptr;
    Ui::SAKMainWindow* ui               = nullptr;
    QMenu* toolsMenu                    = nullptr;    
    QAction *defaultStyleSheetAction    = nullptr;
    SAKUpdateManager* updateManager     = nullptr;
    SAKMoreInformation* moreInformation = nullptr;
    QList<QLabel*> qqLabels;

    QTranslator qtTranslator;
    QTranslator qtBaeTranslator;
    QTranslator sakTranslator;

    const char* appStyleKey = "Universal/appStyle";
    const char* appStylesheetKey = "Universal/appStylesheet";
    /// ----------------------------------------------------
    void AddTab();    
    void AddTool();

    void addTool(QString toolName, QWidget *toolWidget);
    void changeStylesheet(QString styleSheetName);
    void changeAppStyle(QString appStyle);

    void initMenu();
    void initFileMenu();
    void initToolMenu();
    void initOptionMenu();
    void initLanguageMenu();
    void initHelpMenu();
    void initLinksMenu();
    void initStatusBar();


    void installLanguage();
    void addRemovablePage();
    void openIODeviceWindow();
    QWidget *getDebugPage(int type);
    void closeDebugPage(int index);
    void About();
private:
    void createCRCCalculator();
};

#endif // MAINWINDOW_H
