#ifndef CLIENT_TABBAR_H
#define CLIENT_TABBAR_H

#include <QTabBar>

#include "stdint.h"

class QString;

struct ClientTabData {
   ClientTabData(int64_t client_id, int64_t currentProject_id=-1)
   : client_id(client_id)
   , currentProject_id(currentProject_id) {}

   int64_t client_id,
           currentProject_id;
};

Q_DECLARE_METATYPE(ClientTabData*)

class ClientTabBar : public QTabBar {
   Q_OBJECT
public:
   ClientTabBar(QWidget *parent=0);
   ~ClientTabBar();

  /* Eliminate default copy constructor and assignment operator */
   ClientTabBar(const ClientTabBar&)= delete;
   ClientTabBar& operator=(const ClientTabBar&)= delete;

   int addTab(const QString &text, int64_t client_id, int64_t currentProject_id=-1);
   ClientTabData* tabData(int ndx);
   ClientTabData* currentData();
   int64_t tabClient_id(int ndx);
   int64_t tabCurrentProject_id(int ndx);
   int64_t currentClient_id();
   int64_t currentCurrentProject_id();
   void removeTab(int ndx);

 public slots:

   void projectSelected(int64_t client_id, int64_t project_id);
};



#endif

