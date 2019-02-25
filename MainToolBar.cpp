#include <QApplication>
#include <QCursor>
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include <QToolTip>

#include "ClientEditor.h"
#include "Client.h"
#include "configItem.h"
#include "MainToolBar.h"
#include "MainWindow.h"
#include "ProjectChooser.h"
#include "ProjectComboBox.h"
#include "qtimemage.h"
#include "util.h"

static configInt S_GUI_isVisible("GUI_isVisible", 1);

MainToolBar::
MainToolBar(QWidget * parent)
/***********************************************************/
 : QToolBar(parent)
 , _visibleClient_id(-1)
 , _recording_prj_id(-1)
 , _sysTray(NULL)
 , _trayMenu(NULL)
{

   /* When settings will get stored */
   connect(&G, SIGNAL(sig_aboutToSaveSettings()), SLOT(storeSettings()));

   /* Record notifications */
   connect(&G.clientTable, SIGNAL(sig_update(int64_t)), SLOT(client_update(int64_t)));
   connect(&G.clientTable, SIGNAL(sig_update(int64_t)), SLOT(client_remove(int64_t)));

   connect(&G.eventTable, SIGNAL(sig_insert(int64_t)), SLOT(event_insert(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_update(int64_t)), SLOT(event_update(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_remove(int64_t)), SLOT(event_remove(int64_t)));

   connect(&G, SIGNAL(sig_userEventsChangeNotice()), SLOT(userEventsChangeNotice()));

   /******* Create a client menu *********/
   QMenu *menu = new QMenu(this);

   /* Create */
   QAction *create_act= menu->addAction(QIcon::fromTheme("document-new"), "New Client");
   connect(create_act, SIGNAL(triggered()), SLOT(createClient()));

   /* Edit */
   _editClient_act = menu->addAction(QIcon::fromTheme("document-properties"), "Edit Client");
   _editClient_act->setEnabled(false);
   connect(_editClient_act, SIGNAL(triggered()), SLOT(editClient()));

   /* Delete */
   _deleteClient_act = menu->addAction(QIcon::fromTheme("edit-delete"), "Delete Client");
   _deleteClient_act->setEnabled(false);
   connect(_deleteClient_act, SIGNAL(triggered()), SLOT(deleteClient()));

   /* Client menu button */
   QToolButton *tool_btn= new QToolButton(this);
   tool_btn->setMenu(menu);
   tool_btn->setPopupMode(QToolButton::DelayedPopup);
   tool_btn->setDefaultAction(create_act);

   addWidget(tool_btn);

   addSeparator();

   /* Time control buttons */
   _record_act= addAction(QIcon::fromTheme("media-record"), "Charge Project", this, SLOT(Record()));
   _record_act->setEnabled(false);

   _pause_act= addAction(QIcon::fromTheme("media-playback-pause"), "Pause Project", this, SLOT(Pause()));
   _pause_act->setEnabled(false);

   _stop_act= addAction(QIcon::fromTheme("media-playback-stop"), "Stop Project", this, SLOT(Stop()));
   _stop_act->setEnabled(false);

   addSeparator();

   /* Quick project access */
   _project_cb= new ProjectComboBox(this);
   addWidget(_project_cb);
   connect(_project_cb, SIGNAL(activated(int)), SLOT(relayCurrentProject(int)));

   /* Defer the loading of clients from the database until all GUI constructors have been called. */
   QMetaObject::invokeMethod(this, "finish_setup", Qt::QueuedConnection);

   // Keep trying until the tool tray appears
   _trayTimer= new QTimer(this);
   connect(_trayTimer, SIGNAL(timeout()), this, SLOT(systray_install()));
   _trayTimer->setSingleShot(true);
   _trayTimer->setInterval(100);
   _trayTimer->start();

   /* Miscellaneous actions */
   _chargeOther_act= addAction(QIcon::fromTheme("media-record"), "Charge Other Project", this, SLOT(chargeOtherProject()));
}

MainToolBar::
~MainToolBar()
/***********************************************************/
{}

void
MainToolBar::
storeSettings()
/*****************************************************************************
 * Store the main window geometry.
 */
{
  S_GUI_isVisible= isVisible();
}

void
MainToolBar::
sysTrayActivated(QSystemTrayIcon::ActivationReason reason)
/***********************************************************
 * System tray has been activated by user
 */
{


   switch(reason) {
      /*--- Left click to show the context menu ---*/
      case QSystemTrayIcon::Trigger:
         {
            QRect tray_geom(_sysTray->geometry());
            QPoint tray_lrc(tray_geom.x() + tray_geom.width(), tray_geom.y() + tray_geom.height());
            QSize menu_sz= _trayMenu->sizeHint();
            QPoint menu_ulc(tray_lrc.x() - menu_sz.width(), tray_lrc.y());
            _trayMenu->move(menu_ulc);
            _trayMenu->show();
        } break;

      /*---- Right click to charge another project ---*/
      case QSystemTrayIcon::Context:
         _chargeOther_act->trigger();
         break;

      /*--- Start recording if possible ---*/
      case QSystemTrayIcon::DoubleClick:
#if 0
         // FIXME: I'm not sure this ever happens if Trigger is handled
         if(_record_act->isEnabled()) {
            _record_act->triggered();
         }
#endif
         break;

      /*--- Bring up the 'charge other project' dialog ---*/
      case QSystemTrayIcon::MiddleClick:
         // TODO: What is an appropriate behavior?
         break;

      case QSystemTrayIcon::Unknown:
         qWarning("sysTrayActivated() called for Unknown reason!");
         break;
   }
}

void
MainToolBar::
systray_install()
/***********************************************************
 * Try to install the system tray.
 */
{
   /* Have to wait until system tray is available */
   if(!QSystemTrayIcon::isSystemTrayAvailable()) {
      _trayTimer->setInterval(1000);
      _trayTimer->start();
      return;
   }

   _sysTray= new QSystemTrayIcon(QIcon::fromTheme("clock"), this);

   /* Arrange to find out when user activates systray icon */
   connect(_sysTray, &QSystemTrayIcon::activated, this,  &MainToolBar::sysTrayActivated);

   /* Make a menu for left-click on system tray icon */
   _trayMenu= new QMenu;
   QAction *act= _trayMenu->addAction("Show GUI");
   connect(act, &QAction::triggered, this, &MainToolBar::showGUI);

   _trayMenu->addAction(_record_act);
   _trayMenu->addAction(_pause_act);
   _trayMenu->addAction(_stop_act);
   _trayMenu->addAction(_chargeOther_act);
   _trayMenu->addSeparator();

   act= _trayMenu->addAction("Quit");
   connect(act, &QAction::triggered, G.pApp, &QApplication::quit);

   /* Make sure systray icon is correct for the current project */
   syncControls();

   _sysTray->show();

   G.setVisibleTopLevelWindows(S_GUI_isVisible.val());
}

void
MainToolBar::
showGUI()
/*****************************************************************************
 * User wants to see the GUI
 */
{
   if(isVisible()) {
      G.setVisibleTopLevelWindows(false);
   }
   G.setVisibleTopLevelWindows();
}

void
MainToolBar::
finish_setup()
/*****************************************************************************
 * Populate the tab widget with clients.
 */
{
   int ndx_recording= -1;
   _project_cb->populate();

   for(int i= 0; i < _project_cb->count(); ++i) {
      Project *prj= _project_cb->ndx2Project(i);
      Q_ASSERT(prj);
      if(prj->currentState() == Project::RECORDING) {
         ndx_recording= i;
         _recording_prj_id= prj->id();
      }
   }

   if(ndx_recording != -1) {

      _project_cb->setCurrentIndex(ndx_recording);

   }

   syncControls();

}

void
MainToolBar::
client_update(int64_t client_id)
/***********************************************************
 * A client record was updated.
 */
{
   Project *prj;

   for(int i= 0; i < _project_cb->count(); ++i) {

      prj= _project_cb->ndx2Project(i);
      Q_ASSERT(prj);

      /* See if this will effect our combo box text */
      if(client_id == prj->client_id()) {
         _project_cb->setItemText(i, prj->FQLongTitle());
      }
   }
}

void
MainToolBar::
client_remove(int64_t client_id)
/***********************************************************
 * A client record was removed.
 */
{
   Project *prj;

   if(_visibleClient_id == client_id) {
      _visibleClient_id= -1;
   }

   for(int i= 0; i < _project_cb->count(); ++i) {

      prj= _project_cb->ndx2Project(i);
      Q_ASSERT(prj);

      if(prj->id() == _recording_prj_id) {
         _recording_prj_id= -1;
      }

      /* See if this will effect our combo box text */
      if(client_id == prj->client_id()) {
         _project_cb->removeItem(i);
      }
   }

   syncControls();
}

void
MainToolBar::
event_insert(int64_t event_id)
/***********************************************************
 * A event record was inserted.
 */
{
   Event *ev= G.eventTable[event_id];
   Q_ASSERT(ev);
   int64_t prj_id= ev->project_id();

   /* Check to see if this preempts another currently recording project */
   switch(ev->type_enum()) {

      case Event::START: {
         if(_recording_prj_id != -1 && _recording_prj_id != prj_id) {

            Project *recPrj= G.projectTable[_recording_prj_id];
            Q_ASSERT(recPrj);

            /* Pause the old project */
            recPrj->Pause();
         }
         /* We now know to track the new project */
         _recording_prj_id= prj_id;
      } break;

      case Event::PAUSE:
      case Event::STOP: {
         if(_recording_prj_id != -1 && _recording_prj_id == prj_id) {
            _recording_prj_id= -1;
         }
      } break;

   }
}

void
MainToolBar::
event_update(int64_t event_id)
/***********************************************************
 * A event record was
 */
{
   Event *ev= G.eventTable[event_id];
   Q_ASSERT(ev);
   int64_t prj_id= ev->project_id();

   /* May need to cancel currently recording project */
   if(prj_id == _recording_prj_id) {
      Project *prj= G.projectTable[prj_id];
      Q_ASSERT(prj);
      if(prj->currentState() != Project::RECORDING) {
         _recording_prj_id= -1;
      }
   }
}

void
MainToolBar::
event_remove(int64_t event_id)
/***********************************************************
 * A event record was
 */
{
   Event *ev= G.eventTable[event_id];
   Q_ASSERT(ev);

   int64_t prj_id= ev->project_id();
   if(prj_id == _recording_prj_id) {
      Project *prj= G.projectTable[prj_id];
      Q_ASSERT(prj);
      if(prj->currentState() != Project::RECORDING) {
         _recording_prj_id= -1;
      }
   }
}

void
MainToolBar::
setCurrentClient(int64_t client_id)
/***********************************************************
 * This is now the current client.
 */
{
   _visibleClient_id= client_id;
   _editClient_act->setEnabled(_visibleClient_id == -1 ? false : true);
   _deleteClient_act->setEnabled(_visibleClient_id == -1 ? false : true);
}

void
MainToolBar::
setCurrentProject(int64_t prj_id)
/***********************************************************
 * This is now the current project.
 */
{
//J_DBG_FN;
   /* Check the case were the project is already in the QComboBox */
   int ndx= _project_cb->project_id2ndx(prj_id);

   if(-1 == ndx) {

      /* we need to put a new entry into the QComboBox */
      Project *prj= G.projectTable[prj_id];
      Q_ASSERT(prj);

      _project_cb->addProjectSorted(prj);

      ndx= _project_cb->project_id2ndx(prj_id);

   }

   _project_cb->setCurrentIndex(ndx);

   syncControls();
}

void
MainToolBar::
syncControls()
/*****************************************************************************
 * Sync up the controls with the current project.
 */
{
//J_DBG_FN;
   /* Default to shutting everything off */
   _record_act->setEnabled(false);
   _pause_act->setEnabled(false);
   _stop_act->setEnabled(false);
   
   int ndx= _project_cb->currentIndex();
   if(-1 == ndx) {
      _record_act->setText("Charge Project");
      _pause_act->setText("Pause Project");
      _stop_act->setText("Stop Project");
      if(_sysTray) _sysTray->setIcon(QIcon::fromTheme("clock"));
      return;
   }

   int64_t prj_id= _project_cb->ndx2Project_id(ndx);

   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);

   QString title= prj->FQLongTitle();
   _record_act->setText(QString("Start \"%1\"").arg(title));
   _pause_act->setText(QString("Pause \"%1\"").arg(title));
   _stop_act->setText(QString("Stop \"%1\"").arg(title));

   if(_sysTray) {
      
      _sysTray->setToolTip(prj->FQLongTitle());

#if 0
      /* Attempt to fix the black-on-black tooltip issue.
       * https://github.com/csete/gqrx/files/1752642/tool_color.txt
       */
      QPalette p = QToolTip::palette();
//      p.setColor(QPalette::All, QPalette::ToolTipText, Qt::black);
      p.setColor(QPalette::Inactive, QPalette::ToolTipText, Qt::white);
      p.setColor(QPalette::Inactive, QPalette::Window, Qt::black);
      QToolTip::setPalette(p);
#endif
   }

   switch(prj->currentState()) {
      case Project::PAUSED:
         _record_act->setText(QString("Unpause \"%1\"").arg(title));
         _stop_act->setEnabled(true);
         _record_act->setEnabled(true);
         if(_sysTray) _sysTray->setIcon(QIcon::fromTheme("clock"));
         break;

      case Project::STOPPED:
         _record_act->setEnabled(true);
         if(_sysTray) _sysTray->setIcon(QIcon::fromTheme("clock"));
         break;

      case Project::RECORDING:
         _pause_act->setEnabled(true);
         _stop_act->setEnabled(true);
         if(_sysTray) _sysTray->setIcon(QIcon::fromTheme("emblem-colors-red"));
         break;

      case Project::ERROR:
         Q_ASSERT(0);
   }
}

void
MainToolBar::
relayCurrentProject(int ndx)
/*****************************************************************************
 * Publish the new project of interest.
 */
{
//J_DBG_FN;
   syncControls();

   /* No project is currently selected */
   if(ndx == -1) return;

   int64_t prj_id= _project_cb->ndx2Project_id(ndx);

   /* Tell everyone else to show this project */
   emit sig_showProject(prj_id);


}

void
MainToolBar::
userEventsChangeNotice()
/*****************************************************************************
 * The user has moved some events around, so we need to adapt.
 */
{
//   J_DBG_FN << "LOOK!";
   // TODO: search for the new recording project?
   _recording_prj_id= -1;
   syncControls();
}

void
MainToolBar::
Record()
/*****************************************************************************
 * Time is billable.
 */
{
   int ndx= _project_cb->currentIndex();
   Project *prj= _project_cb->ndx2Project(ndx);
   Q_ASSERT(prj);

   switch(prj->currentState()) {

      case Project::STOPPED:
      case Project::PAUSED:
         prj->Start();
         break;

      default:
         Q_ASSERT(0);
   }

   syncControls();

   /* GUI will update from G.eventTable signals */

}

void
MainToolBar::
Pause()
/*****************************************************************************
 * Time is not billable.
 */
{
   int ndx= _project_cb->currentIndex();
   Project *prj= _project_cb->ndx2Project(ndx);
   Q_ASSERT(prj);

   /* Can't pause if we aren't currently recording */
   // FIXME: When events get moved, this may not be true!
   Q_ASSERT(prj->currentState() == Project::RECORDING);
   if(prj->id() == _recording_prj_id) {
      _recording_prj_id= -1;
   }
   prj->Pause();

   syncControls();
   /* GUI will update from G.eventTable signals */
}

void
MainToolBar::
Stop()
/*****************************************************************************
 * The current project will be stopped.
 */
{
   int ndx= _project_cb->currentIndex();
   Project *prj= _project_cb->ndx2Project(ndx);
   Q_ASSERT(prj);

   switch(prj->currentState()) {

      case Project::RECORDING:
      case Project::PAUSED:
         prj->Stop();
         break;

      default:
        Q_ASSERT(0);
   }


   syncControls();
   /* GUI will update from G.eventTable signals */
}

void
MainToolBar::
chargeOtherProject()
/*****************************************************************************
 * Let user choose a new project to charge
 */
{
   int rtn;
   /* Run the picker modally */
   ProjectChooser pc("Choose a Project to charge");
   pc.setAttribute(Qt::WA_QuitOnClose, false);
   rtn = pc.exec();

   if (rtn == QDialog::Accepted) {
      int64_t prj_id= pc.selectedProject_id();
      setCurrentProject(prj_id);
      Record();
   }


}
void
MainToolBar::
createClient()
/*****************************************************************************
 * Create a new client
 */
{
   int rtn;
   Client client;
   /* TODO: Pull default information from a config file */
   client.set_default_rate(10000);
   client.set_default_charge_quantum(10);
   client.set_acronym("New");
   client.set_name("New Client Record");

   /* Run the client editor modally */
   ClientEditor ce(this, &client);
   rtn = ce.exec();

   if (rtn == QDialog::Accepted) {
      ce.assign(&client);
      if (client.insert()) Q_ASSERT(0);
      /* clientTable will emit a signal, and client_insert() will be called. */
   }
}

void
MainToolBar::
editClient()
/*****************************************************************************
 * Edit the current client
 */
{
   int rtn;
   Q_ASSERT(_visibleClient_id != -1);

   Client *cl= G.clientTable[_visibleClient_id];
   Q_ASSERT(cl);
   /* Run the client editor modally */
   ClientEditor ce(this, cl);
   rtn = ce.exec();

   /* Only update if user accepted the changes */
   if (rtn != QDialog::Accepted) return;

   /* Assign new values to our copy */
   ce.assign(cl);

   if (cl->update()) Q_ASSERT(0);
   /* clientTable will emit a signal, and client_update() will be called. */
}

void
MainToolBar::
deleteClient()
/*****************************************************************************
 * Delete the current client
 */
{
   Q_ASSERT(_visibleClient_id != -1);

   Client *cl= G.clientTable[_visibleClient_id];
   Q_ASSERT(cl);

   int rtn;

   {				/* Ask user if they are really sure */
      QString name;
      if(cl->name(&name)) Q_ASSERT(0);
      QString question = QString("Are you sure you want to delete client:\n\"%1\"?").arg(name);

      rtn = QMessageBox::warning(this,
				 "Delete Client",
				 question,
				 QMessageBox::Ok | QMessageBox::Cancel,
				 QMessageBox::Cancel);

   }

   if (rtn != QMessageBox::Ok) return;

   _visibleClient_id= -1;

   G.pApp->setOverrideCursor(QCursor(Qt::WaitCursor));
   if (cl->remove()) Q_ASSERT(0);
   G.pApp->restoreOverrideCursor();
   // G.clientTable will emit a signal, and GUI() will get adjusted.

}

