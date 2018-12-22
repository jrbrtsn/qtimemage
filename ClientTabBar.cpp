#include <QVariant>
#include <QString>

#include "qtimemage.h"
#include "ClientTabBar.h"


ClientTabBar::
ClientTabBar(QWidget *parent)
/*****************************************************************************
 * Constructor
 */
: QTabBar(parent)
{}

ClientTabBar::
~ClientTabBar()
/*****************************************************************************
 * Find the QTabWidget index for the client of prj_id project, or -1;
 */
{}

int
ClientTabBar::
addTab(const QString &text, int64_t client_id, int64_t currentProject_id)
/*****************************************************************************
 * Add a tab with a ClientTabData object attached.
 */
{
   ClientTabData *td= new ClientTabData(client_id, currentProject_id);
   int ndx= QTabBar::addTab(text);
   QTabBar::setTabData(ndx, QVariant::fromValue(td));
   return ndx;
}

ClientTabData*
ClientTabBar::
tabData(int ndx)
/*****************************************************************************
 * Retrieve ClientTabData object attached.
 */
{
   if(-1 == ndx) return NULL;
   QVariant qv= QTabBar::tabData(ndx);
   if(!qv.isValid()) return NULL;
   ClientTabData *td= qv.value<ClientTabData*>();
   Q_ASSERT(td);
   return td;
}

ClientTabData*
ClientTabBar::
currentData()
/*****************************************************************************
 * Retrieve ClientTabData object attached.
 */
{
   return tabData(currentIndex());
}

int64_t
ClientTabBar::
tabClient_id(int ndx)
/*****************************************************************************
 * Return the client_id for ndx, or -1;
 */
{
   ClientTabData *td= tabData(ndx);
   return td ? td->client_id : -1;
}

int64_t
ClientTabBar::
tabCurrentProject_id(int ndx)
/*****************************************************************************
 * Return the currentProject_id for ndx, or -1;
 */
{
   ClientTabData *td= tabData(ndx);
   return td ? td->currentProject_id : -1;
}

int64_t
ClientTabBar::
currentClient_id()
/*****************************************************************************
 * Return the current client_id, or -1;
 */
{
   return tabClient_id(currentIndex());
}

int64_t
ClientTabBar::
currentCurrentProject_id()
/*****************************************************************************
 * Return the current currentProject_id, or -1;
 */
{
   return tabCurrentProject_id(currentIndex());
}

void
ClientTabBar::
removeTab(int ndx)
/*****************************************************************************
 * Function overloaded to free ClientTabData object.
 */
{
   ClientTabData *td= tabData(ndx);
   if(td) delete td;
   QTabBar::removeTab(ndx);
}

void
ClientTabBar::
projectSelected(int64_t client_id, int64_t project_id)
/*****************************************************************************
 * Function sets value in ClientTabData.
 */
{
//J_DBG_FN;
   ClientTabData *data;
   for(int i= 0; i < count(); ++i) {
      data= tabData(i);
      if(data->client_id == client_id) {
         data->currentProject_id= project_id;
//J_DBG_FN << "ndx= " << i << ", data= " << data << " { client_id= " << data->client_id << ", currentProject_id= " << data->currentProject_id << " }";
         break;
      }
   }
}
