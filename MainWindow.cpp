#include <QCloseEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QToolBar>
#include <QVBoxLayout>

#include "Client.h"
#include "ClientPage.h"
#include "ClientTabBar.h"
#include "configItem.h"
#include "MainToolBar.h"
#include "MainWindow.h"
#include "qtimemage.h"
#include "util.h"

static configRect S_geometry("MainWindow_geom", 20, 20, 600, 400);
static configInt S_tabNdx("MainWindow_clientTab_ndx", 0);
static configIntVec S_project_ids("ClientTab_project_ids");
static configIntVec S_client_ids("ClientTab_client_ids");


MainWindow::
MainWindow(QWidget * parent)
/***********************************************************/
 : QMainWindow(parent)
{

   /* Set window title */
   setWindowTitle(QString("%1 - v%2").arg(G.programTitle).arg(G.programVersion));

   /* Listen for Global setVisible signals */
   connect(&G, SIGNAL(sig_setVisibleTopLevelWindows(bool)), SLOT(setVisible(bool)));

   /* Geometry management */
   setGeometry(S_geometry);
   connect(&G, SIGNAL(sig_aboutToSaveSettings()), SLOT(storeSettings()));

   /* Record notifications */
   connect(&G.clientTable, SIGNAL(sig_insert(int64_t)), SLOT(client_insert(int64_t)));
   connect(&G.clientTable, SIGNAL(sig_update(int64_t)), SLOT(client_update(int64_t)));
   connect(&G.clientTable, SIGNAL(sig_remove(int64_t)), SLOT(client_remove(int64_t)));

   connect(&G.eventTable, SIGNAL(sig_insert(int64_t)), SLOT(event_insert(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_update(int64_t)), SLOT(event_update(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_remove(int64_t)), SLOT(event_remove(int64_t)));

   /* Toolbar for the top of the window */
   _main_tb = new MainToolBar(this);
   addToolBar(Qt::TopToolBarArea, _main_tb);

   /* Notify the toolbar when there is a new current client. */
   connect(this, SIGNAL(sig_currentClient(int64_t)), _main_tb, SLOT(setCurrentClient(int64_t)));

   /* Show project when told to */
   connect(_main_tb, SIGNAL(sig_showProject(int64_t)), SLOT(showProject(int64_t)));

   { /************** Central Widget */
      QWidget *w= new QWidget;
      setCentralWidget(w);
      QVBoxLayout *vbl= new QVBoxLayout(w);

      _client.tb= new ClientTabBar(w);
      vbl->addWidget(_client.tb);
      _client.tb->setExpanding(false);
      _client.tb->setMovable(true);
      connect(_client.tb, SIGNAL(tabMoved(int,int)), SLOT(clientTabMoved()));

      _client.pg= new ClientPage(w);
      vbl->addWidget(_client.pg);
      connect(_client.pg, SIGNAL(sig_doubleClickedProject(int64_t)), _main_tb, SLOT(setCurrentProject(int64_t)));
      connect(_client.pg, SIGNAL(sig_projectSelected(int64_t,int64_t)), _client.tb, SLOT(projectSelected(int64_t,int64_t)));
   }

   setStatusBar(new QStatusBar);

   /* Defer the loading of clients from the database until all GUI constructors have been called. */
   QMetaObject::invokeMethod(this, "populate_clients", Qt::QueuedConnection);

}

MainWindow::
~MainWindow()
/***********************************************************/
{}

void
MainWindow::
closeEvent(QCloseEvent *event)
/*****************************************************************************
 * Overloaded function possibly intercepts the close event.
 */
{
   if(QSystemTrayIcon::isSystemTrayAvailable()) {
      G.setVisibleTopLevelWindows(false);
      event->ignore();
   }
}

void
MainWindow::
clientTabMoved()
/*****************************************************************************
 * Client tabs got shuffled.
 */
{
   G.saveSettings();
}

int
MainWindow::
findProjectClientNdx(int64_t prj_id)
/*****************************************************************************
 * Find the QTabWidget index for the client of prj_id project, or -1;
 */
{
   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);
   int64_t client_id= prj->client_id();

   return findClientNdx(client_id);
}

void
MainWindow::
showProject(int64_t prj_id)
/*****************************************************************************
 * Show the project represented by prj_id.
 */
{
   int ndx= findProjectClientNdx(prj_id);

   if(-1 == ndx) return;

   _client.tb->setCurrentIndex(ndx);
   _client.pg->showProject(prj_id);
}

void
MainWindow::
currentClientChanged(int ndx)
/*****************************************************************************
 * Field the signal from the client QTabBar, broadcast our signal.
 */
{
   int64_t client_id= -1,
           currentProject_id= -1;

   if(-1 != ndx) {
      client_id= _client.tb->tabClient_id(ndx);
      currentProject_id= _client.tb->tabCurrentProject_id(ndx);
   }

   _client.pg->setClient(client_id, currentProject_id);
   emit sig_currentClient(client_id);
}

void
MainWindow::
populate_clients()
/*****************************************************************************
 * Populate the tab widget with clients.
 */
{
   Q_ASSERT(_client.tb->count() == 0);

   QVector<int64_t> client_id_vec;

   /* Fetch all client records */
   int rtn = G.clientTable.fetchAll(client_id_vec, "ORDER BY acronym ASC");
   if (rtn < 0) qFatal("Client_Table::fetchAll() failed!");

   /* A tab for each client */
   Client *cl;
//   Project *prj;
   int64_t client_id, prj_id;
   QString acronym;

   /* Put Client tabs in order of configuration file first */
   for(int i= 0; i < S_client_ids.count(); ++i) {

      client_id= S_client_ids[i];

      /* Search through results from db query */
      for(int j= 0; j < client_id_vec.count(); ++j) {

         if(client_id_vec[j] != client_id) continue;

         /* Remove this vector item since it has now been addressed */
         client_id_vec.remove(j);

         cl= G.clientTable[client_id];
         Q_ASSERT(cl);
         acronym= cl->acronym();
         prj_id= S_project_ids.count() > i ? S_project_ids[i] : -1;
         _client.tb->addTab(acronym, client_id, prj_id);
         _client.tb->setTabIcon(i, cl->currentStateIcon());
      }
   }

   /* Put any remaining clients in other tabs */
   for (int i = 0; i < client_id_vec.count(); ++i) {
      client_id= client_id_vec[i];
      cl= G.clientTable[client_id];
      Q_ASSERT(cl);
      acronym= cl->acronym();
      _client.tb->addTab(acronym, client_id, -1);
      _client.tb->setTabIcon(i, cl->currentStateIcon());
   }

   int ndx= S_tabNdx.val();
   _client.tb->setCurrentIndex(ndx);
   currentClientChanged(ndx);
   /* Don't connect signal until we get to here */
   connect(_client.tb, SIGNAL(currentChanged(int)), SLOT(currentClientChanged(int)));

}

void
MainWindow::
client_insert(int64_t id)
/*****************************************************************************
 * A client record was inserted in client_tbl
 */
{
   Client *cl= G.clientTable[id];
   Q_ASSERT(cl);
   /* Put in a new page for the new record */
   QString acronym= cl->acronym();
   int ndx= _client.tb->addTab(acronym, id);
   _client.tb->setCurrentIndex(ndx);
   G.saveSettings();
}

int64_t
MainWindow::
currentClient_id()
/*****************************************************************************
 * Return the address of the current client, or NULL if there is none.
 */
{
   return _client.tb->currentClient_id();
}

int
MainWindow::
findClientNdx(int64_t client_id)
/*****************************************************************************
 * Find the QTabBar index representing the supplied Client.
 */
{
   for(int i= 0; i < _client.tb->count(); ++i) {
      if(_client.tb->tabClient_id(i) == client_id) return i;
   }

   return -1;
}

void
MainWindow::
client_update(int64_t client_id)
/*****************************************************************************
 * A client record was updated in client_tbl
 */
{
   int ndx= findClientNdx(client_id);

   /* May not be our Client */
   if(ndx == -1) return;

   Client *cl= G.clientTable[client_id];
   Q_ASSERT(cl);

   QString acronym= cl->acronym();
   _client.tb->setTabText(ndx, acronym);
}

void
MainWindow::
client_remove(int64_t client_id)
/*****************************************************************************
 * A client record was removed from client_tbl
 */
{
   int ndx= findClientNdx(client_id);

   /* May not be our Client */
   if(ndx == -1) return;

   _client.tb->removeTab(ndx);

}

void
MainWindow::
event_insert(int64_t event_id)
/*****************************************************************************
 * A event record was inserted in event_tbl
 */
{
   Event *ev= G.eventTable[event_id];
   Q_ASSERT(ev);
   int64_t prj_id= ev->project_id();


   int ndx;

   ndx= findProjectClientNdx(prj_id);
   if(ndx == -1) return;

   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);

   Client *cl= G.clientTable[prj->client_id()];
   Q_ASSERT(cl);

   _client.tb->setTabIcon(ndx, cl->currentStateIcon());

}

void
MainWindow::
event_update(int64_t event_id)
/*****************************************************************************
 * A event record was updated in event_tbl
 */
{
   event_insert(event_id);
}

void
MainWindow::
event_remove(int64_t event_id)
/*****************************************************************************
 * A event record was removed in event_tbl
 */
{
   event_insert(event_id);
}

void
MainWindow::
storeSettings()
/*****************************************************************************
 * Store the main window geometry.
 */
{
  S_geometry= geometry();
  S_tabNdx.set_val(_client.tb->currentIndex());

  S_project_ids.resize(_client.tb->count());
  S_client_ids.resize(_client.tb->count());

  for(int i= 0; i < _client.tb->count(); ++i) {
     S_project_ids[i]= _client.tb->tabCurrentProject_id(i);
     S_client_ids[i]= _client.tb->tabClient_id(i);
  }

}

void
MainWindow::
show_errMsg(const QString& msg)
/*****************************************************************************
 * Display an error message to the user.
 */
{
  QMessageBox::warning(this, G.programTitle, msg);
}

