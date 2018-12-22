#ifndef QTIMEMAGE_H
#define QTIMEMAGE_H

#include <QSqlDatabase>
#include <QString>
#include <QObject>
#include <QDir>
#include <QPointer>

#include "Client.h"
#include "Project.h"

class configItem;
class QApplication;
class MainWindow;

class Global : public QObject {

   Q_OBJECT

 public:
   Global();
   ~Global();

   static void msgHandler(QtMsgType type, const char *msg);

   void register_configItem(const char *key, configItem *item);

  signals:
   void sig_aboutToSaveSettings();
   void sig_setVisibleTopLevelWindows(bool tf);
   void sig_userEventsChangeNotice();

  public slots:
   void loadSettings();
   void saveSettings();
   void setVisibleTopLevelWindows(bool tf=true);
   void userEventsChangeNotice();

 public:
   QDir dataDir;

   QString programTitle,
           programVersion;
   
   QApplication *pApp;
   QPointer<MainWindow> pMw;

    /* Access to our database tables */
   QSqlDatabase db;
   Client_Table clientTable;
   Project_Table projectTable;
   Event_Table eventTable;

};

extern Global G;


#endif				// QTIMEMAGE_H
