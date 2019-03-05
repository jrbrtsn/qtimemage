
#include <QSplitter>
#include <QTimer>
#include <QMenu>
#include <QGroupBox>
#include <QBoxLayout>
#include <QLabel>
#include <QToolBox>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QMessageBox>
#include <QShowEvent>
#include <QDate>

#include "qtimemage.h"
#include "ProjectTreeWidget.h"
#include "Report.h"
#include "Project.h"
#include "ProjectEditor.h"
#include "ProjectPage.h"
#include "Client.h"
#include "configItem.h"
#include "util.h"
#include "WeekChooser.h"
#include "MonthChooser.h"
#include "ClientPage.h"

static configIntVec S_spltSettings("ClientPage_splitter_settings");
static configIntVec S_projectHdrSettings("ClientPage_ProjectsTree_header_settings");

ClientPage::
ClientPage(QWidget *parent)
/******************************************************************************************
 * Constructor
 */
: QWidget(parent),
_do_storeSettings(false)
{
   _client.id= -1;
   /* Geometry management */
   connect(&G, SIGNAL(sig_aboutToSaveSettings()), SLOT(storeSettings()));

   /* Record notifications */
   connect(&G.projectTable, SIGNAL(sig_insert(int64_t)), SLOT(project_insert(int64_t)));
   connect(&G.projectTable, SIGNAL(sig_update(int64_t)), SLOT(project_update(int64_t)));
   connect(&G.projectTable, SIGNAL(sig_remove(int64_t)), SLOT(project_remove(int64_t)));

   connect(&G.eventTable, SIGNAL(sig_insert(int64_t)), SLOT(refreshTimes()));
   connect(&G.eventTable, SIGNAL(sig_update(int64_t)), SLOT(refreshTimes()));
   connect(&G.eventTable, SIGNAL(sig_remove(int64_t)), SLOT(refreshTimes()));

   { /******* Create a project menu *********/
      _project.menu = new QMenu(this);

      /* Create */
      _project.create_act= _project.menu->addAction(QIcon::fromTheme("document-new"), "New Project");
      connect(_project.create_act, SIGNAL(triggered()), SLOT(createProject()));

      /* Create Sub */
      _project.createSubproject_act= _project.menu->addAction(QIcon::fromTheme("contact-new"), "New Subproject");
      _project.createSubproject_act->setEnabled(false);
      connect(_project.createSubproject_act, SIGNAL(triggered()), SLOT(createSubproject()));

      /* Edit */
      _project.edit_act = _project.menu->addAction(QIcon::fromTheme("document-properties"), "Edit Project");
      _project.edit_act->setEnabled(false);
      connect(_project.edit_act, SIGNAL(triggered()), SLOT(editProject()));

      /* Delete */
      _project.delete_act = _project.menu->addAction(QIcon::fromTheme("edit-delete"), "Delete Project");
      _project.delete_act->setEnabled(false);
      connect(_project.delete_act, SIGNAL(triggered()), SLOT(deleteProject()));
   }

   { /******* Create a client report menu *********/
      _client.report.menu = new QMenu(this);

      /* Weekly */
      _client.report.weekly_act= _client.report.menu->addAction("Weekly Report");
      connect(_client.report.weekly_act, SIGNAL(triggered()), SLOT(weeklyReport()));

      /* Monthly */
      _client.report.monthly_act= _client.report.menu->addAction("Monthly Report");
      connect(_client.report.monthly_act, SIGNAL(triggered()), SLOT(monthlyReport()));

      /* Custom */
      _client.report.custom_act= _client.report.menu->addAction("Custom Report");
      connect(_client.report.custom_act, SIGNAL(triggered()), SLOT(customReport()));
   }

   /******** Static GUI components **********/
   QVBoxLayout *vbl= new QVBoxLayout(this);
   
   { /* Client area */
      QLabel *lbl;

      /* Client's name will be shown as the title of the QGroupBox */
      _client.gb= new QGroupBox(this);
      _client.gb->setAlignment(Qt::AlignHCenter);
      vbl->addWidget(_client.gb);

      QVBoxLayout *vbl= new QVBoxLayout;
      _client.gb->setLayout(vbl);

      { /* 1st row */
         QHBoxLayout *hbl= new QHBoxLayout;
         vbl->addLayout(hbl);

         /* Project menu button */
         QToolButton *tool_btn= new QToolButton(_client.gb);
         tool_btn->setMenu(_project.menu);
         tool_btn->setPopupMode(QToolButton::DelayedPopup);
         tool_btn->setDefaultAction(_project.create_act);
         hbl->addWidget(tool_btn);

         hbl->addStretch(10);

         /* Rate for client */
         lbl= new QLabel("<b>Base Rate:</b> $", _client.gb);
         hbl->addWidget(lbl);
         _client.rate_lbl= new QLabel(_client.gb);
         hbl->addWidget(_client.rate_lbl);

         hbl->addStretch(10);

         /* Default charge quantum for client */
         lbl= new QLabel("<b>Min. Billing (min):</b>", _client.gb);
         hbl->addWidget(lbl);
         _client.charge_quantum_lbl= new QLabel(_client.gb);
         hbl->addWidget(_client.charge_quantum_lbl);

         hbl->addStretch(10);

         tool_btn= new QToolButton(_client.gb);
         tool_btn->setMenu(_client.report.menu);
//         tool_btn->setPopupMode(QToolButton::DelayedPopup);
         tool_btn->setDefaultAction(_client.report.weekly_act);
         hbl->addWidget(tool_btn);
      }

      { /* 2nd row */
         QHBoxLayout *hbl= new QHBoxLayout;
         vbl->addLayout(hbl);

         lbl= new QLabel("<b>Total Hours for Today:</b>", _client.gb);
         hbl->addWidget(lbl);
         _client.timeToday_lbl= new QLabel(_client.gb);
         hbl->addWidget(_client.timeToday_lbl);

         hbl->addStretch(10);

         lbl= new QLabel("<b>This Week:</b>", _client.gb);
         hbl->addWidget(lbl);
         _client.timeWeek_lbl= new QLabel(_client.gb);
         hbl->addWidget(_client.timeWeek_lbl);

         hbl->addStretch(10);

         lbl= new QLabel("<b>This Month:</b>", _client.gb);
         hbl->addWidget(lbl);
         _client.timeMonth_lbl= new QLabel(_client.gb);
         hbl->addWidget(_client.timeMonth_lbl);

         hbl->addStretch(10);

         lbl= new QLabel("<b>This Year:</b>", _client.gb);
         hbl->addWidget(lbl);
         _client.timeYear_lbl= new QLabel(_client.gb);
         hbl->addWidget(_client.timeYear_lbl);

         hbl->addStretch(10);
      }

   }

   _splt= new QSplitter(this);
   _splt->setOrientation(Qt::Vertical);
   vbl->addWidget(_splt);

   { /* Tree view of projects */
      QWidget *w= new QWidget;
      _splt->addWidget(w);
      QHBoxLayout *hbl= new QHBoxLayout(w);
      _project.trw= new ProjectTreeWidget(w);
      hbl->addWidget(_project.trw);

      _project.trw->setMenu(_project.menu);
      /* Relay the signal out */
      connect(_project.trw, SIGNAL(sig_doubleClickedProject(int64_t)), SIGNAL(sig_doubleClickedProject(int64_t)));
      connect(_project.trw, SIGNAL(itemSelectionChanged()), SLOT(projectSelectionChanged()));

   }

   { /* Project page */
      _project.page= new ProjectPage(this);
      _splt->addWidget(_project.page);
   }

   /* Arrange for updates in the future */
   connect(&G.clientTable, SIGNAL(sig_update(int64_t)), SLOT(client_update(int64_t)));

   /* Find out when project gets selected */
   connect(_project.trw, SIGNAL(itemSelectionChanged()), SLOT(setCurrentItem()));

   _refreshTimer= new QTimer(this);
   _refreshTimer->setInterval(180*1000);
   connect(_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimes()));
}

ClientPage::
~ClientPage()
/******************************************************************************************
 * Destructor
 */
{
}

void
ClientPage::
projectSelectionChanged()
/******************************************************************************************
 * The project selection has changed.
 */
{
//J_DBG_FN;
   emit sig_projectSelected(_client.id, _project.trw->selectedProject_id());
}

void
ClientPage::
refreshTimes()
/******************************************************************************************
 * Refresh the project times
 */
{
   QDateTime dtNow,
             dtBeginDay,
             dtBeginWeek,
             dtBeginMonth,
             dtBeginYear;

   dtNow= QDateTime::currentDateTime();
   /* Initialize to the current time */
   QDate currDate= dtNow.date(),
         beginMonthDate;

   dtBeginDay.setDate(currDate);
   dtBeginWeek.setDate(currDate.addDays(-currDate.dayOfWeek()+1));

   beginMonthDate= currDate.addDays(-currDate.day()+1);
   dtBeginMonth.setDate(beginMonthDate);

   dtBeginYear.setDate(beginMonthDate.addMonths(-beginMonthDate.month()+1));

#if 0
   J_DBG_FN << "beginDay= " << dtBeginDay.toString();
   J_DBG_FN << "beginWeek= " << dtBeginWeek.toString();
   J_DBG_FN << "beginMonthDate= " << dtBeginMonth.toString();
   J_DBG_FN << "beginYear= " << dtBeginYear.toString();
#endif

   /* Put these here to avoid repetition in reporting functions */
   ClientReport dayRpt(_client.id, dtBeginDay, dtNow),
                wkRpt(_client.id, dtBeginWeek, dtNow),
                moRpt(_client.id, dtBeginMonth, dtNow),
                yrRpt(_client.id, dtBeginYear, dtNow);

   _project.trw->refreshTimes(dayRpt, wkRpt, moRpt, yrRpt);

   int decihours;

   decihours= dayRpt.cumulativeDecihours();
   _client.timeToday_lbl->setText(decihour2hrStr(decihours));

   decihours= wkRpt.cumulativeDecihours();
   _client.timeWeek_lbl->setText(decihour2hrStr(decihours));

   decihours= moRpt.cumulativeDecihours();
   _client.timeMonth_lbl->setText(decihour2hrStr(decihours));

   decihours= yrRpt.cumulativeDecihours();
   _client.timeYear_lbl->setText(decihour2hrStr(decihours));

}

void
ClientPage::
showEvent(QShowEvent *event)
/*******************************************************************************************
 * Reimplementation of QWidget function.
 */
{
   if(event->type() == QEvent::Show) {

      { /* Splitter settings */
         QList<int> lst;
         for(int i= 0; i < S_spltSettings.count() && i < _splt->count(); ++i) {
            lst.push_back(S_spltSettings[i]);
         }
         _splt->setSizes(lst);
      }

      /* Project QTreeWidget settings */
      for(int i= 0; i < _project.trw->columnCount() && i < S_projectHdrSettings.count(); ++i) {
        _project.trw->setColumnWidth(i, S_projectHdrSettings[i]);
      }

      refreshTimes();
      _refreshTimer->start();
      _do_storeSettings= true;
   }

   /* Call the parent's showEvent() */
   QWidget::showEvent(event);
}

void
ClientPage::
hideEvent(QHideEvent * event)
/*******************************************************************************************
 * Reimplementation of QWidget function.
 */
{
   if(event->type() == QEvent::Hide) {
      storeSettings();
      _refreshTimer->stop();
      _do_storeSettings= false;
   }

   /* Call parent's function */
   QWidget::hideEvent(event);
}

void
ClientPage::
storeSettings()
/*****************************************************************************
 * Store settings if we should.
 */
{
   if(!_do_storeSettings) return;

   { /* Splitter */
      QList<int> lst= _splt->sizes();
      S_spltSettings.resize(lst.count());
      for(int i= 0; i < lst.count(); ++i) {
         S_spltSettings[i]= lst[i];
      }
   }

   /* project QTreeWidget */
   S_projectHdrSettings.resize(_project.trw->columnCount());
   for(int i= 0; i < _project.trw->columnCount(); ++i) {
      S_projectHdrSettings[i]= _project.trw->columnWidth(i);
   }

}

void
ClientPage::
setClient(int64_t client_id, int64_t currentProject_id)
/******************************************************************************************
 * Set display to the supplied client, which may be -1
 */
{
   /* client_update() needs this to work correctly */
   _client.id= client_id;

   client_update(client_id);

   if(_client.id != -1) {
      populate_projects();
      refreshTimes();
      if(currentProject_id != -1) {
         showProject(currentProject_id);
      }
   }
   _project.page->setProject(currentProject_id);
}

int64_t
ClientPage::
currentProject_id()
/******************************************************************************************
 * Return the current project_id, or -1
 */
{
   return _project.trw->currentProject_id();
}

void
ClientPage::
showProject(int64_t prj_id)
/*****************************************************************************
 * Show the project represented by prj_id
 */
{
   if(-1 == prj_id) return;
   
   ProjectTreeWidgetItem *item= _project.trw->findItem(prj_id);
   if(!item) return;
   _project.trw->setCurrentItem(item);
}

void
ClientPage::
setBillingProject(int64_t prj_id)
/*****************************************************************************
 * This will now be the billing project.
 */
{
   ProjectTreeWidgetItem *item= _project.trw->findItem(prj_id);
   Q_ASSERT(item);
   item->setIcon(0, QIcon::fromTheme("emblem-colors-red"));
}

void
ClientPage::
client_update(int64_t client_id)
/*****************************************************************************
 * G.clientTable signalled that a record was updated.
 */
{
   if(client_id != _client.id) return;

   if(_client.id != -1) {

      Client *cl= G.clientTable[_client.id];
      Q_ASSERT(cl);

      _client.gb->setTitle(cl->name());

      _client.rate_lbl->setText(cent2dollarStr(cl->default_rate()));

      int32_t dflt_charge_quantum= cl->default_charge_quantum();
      _client.charge_quantum_lbl->setText(QString("%1").arg(dflt_charge_quantum));

   } else {

      _client.gb->setTitle("> Please Create a Client <");
      _client.rate_lbl->setText("N/A");
      _client.charge_quantum_lbl->setText("N/A");

   }
}


void
ClientPage::
setCurrentItem()
/*****************************************************************************
 * Make adjustments for the QTreeWidget's current item, if any.
 */
{
   int64_t prj_id= currentProject_id();

   if(prj_id != -1) {

      _project.page->setProject(prj_id);
      _project.createSubproject_act->setEnabled(true);
      _project.edit_act->setEnabled(true);
      _project.delete_act->setEnabled(true);

   } else {

      _project.page->setProject(-1);
      _project.createSubproject_act->setEnabled(false);
      _project.edit_act->setEnabled(false);
      _project.delete_act->setEnabled(false);
   }

}

void
ClientPage::
populate_projects()
/*****************************************************************************
 * Populate the toolbox widget with projects.
 */
{
   _project.trw->setClient(_client.id);

   if(_project.trw->currentItem()) {
      _project.createSubproject_act->setEnabled(true);
      _project.edit_act->setEnabled(true);
      _project.delete_act->setEnabled(true);
   }
}

void
ClientPage::
createProject()
/***********************************************************
 * Create a new prj record, and begin editing
 */
{
   int rtn;
   Project prj;
   /* TODO: Pull default information from a config file */
   Client *cl= G.clientTable[_client.id];
   Q_ASSERT(cl);
   prj.set_rate(cl->default_rate());
   prj.set_charge_quantum(cl->default_charge_quantum());
   prj.set_client_id(cl->id());
   prj.set_title("New Project Record");

   /* Run the prj editor modally */
   ProjectEditor pe(this, &prj);
   rtn = pe.exec();

   if (rtn == QDialog::Accepted) {
      pe.assign(&prj);
      if (prj.insert()) Q_ASSERT(0);
   }
}

void
ClientPage::
createSubproject()
/***********************************************************
 * Create a new Project record as a child of the current project record, and begin editing
 */
{
   int rtn;

   Q_ASSERT(_client.id != -1);

   int64_t prj_id= currentProject_id();
   Q_ASSERT(prj_id != -1);
   Project *parent= G.projectTable[prj_id];
   Q_ASSERT(parent);

   /* Now get a project object to work with */
   Project prj;

   prj.set_project_id(parent->id());
   prj.set_rate(parent->rate());
   prj.set_charge_quantum(parent->charge_quantum());
   prj.set_client_id(_client.id);
   prj.set_title("New Subproject Record");

   /* Run the prj editor modally */
   ProjectEditor pe(this, &prj);
   rtn = pe.exec();

   if (rtn == QDialog::Accepted) {
      pe.assign(&prj);
      if (prj.insert()) Q_ASSERT(0);
   }
}

void
ClientPage::
editProject()
/***********************************************************
 * edit the current prj record.
 */
{
   int rtn;
   Project *prj= _project.trw->currentProject();
   Q_ASSERT(prj);

   /* Run the prj editor modally */
   ProjectEditor pe(this, prj);
   rtn = pe.exec();

   if (rtn == QDialog::Accepted) {
      pe.assign(prj);
      if (prj->update()) Q_ASSERT(0);
      /* signal emitted from table should take care of GUI changes */
   }
}

void
ClientPage::
deleteProject()
/***********************************************************
 * Delete the current prj record.
 */
{

   Project *prj= _project.trw->currentProject();
   Q_ASSERT(prj);

   int rtn;

   /* Ask user if they are really sure */
   QString title= prj->title();
   QString question = QString("Are you sure you want to delete project:\n\"%1\"?").arg(title);

   rtn = QMessageBox::warning(this,
		              "Delete Project Record",
			      question,
			      QMessageBox::Ok | QMessageBox::Cancel,
			      QMessageBox::Cancel);


   if (rtn != QMessageBox::Ok) return;

   if (prj->remove()) Q_ASSERT(0);
   // G.projectTable will emit a signal, and project_remove() will get called.
}

void
ClientPage::
project_insert(int64_t prj_id)
/***********************************************************
 * A project record was inserted.
 */
{
   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);

   /* If this isn't our project, ignore it */
   if(prj->client_id() != _client.id) return;

   /* Add a new entry in the ProjectTreeWidget */
   ProjectTreeWidgetItem *item= _project.trw->addProject(prj_id);

   /* Index to that project */
   _project.trw->setCurrentItem(item);
}

void
ClientPage::
project_update(int64_t prj_id)
/***********************************************************
 * Delete the current prj record.
 */
{
   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);

   /* If this isn't our project, ignore it */
   if(prj->client_id() != _client.id) return;

   ProjectTreeWidgetItem *item= _project.trw->findItem(prj_id);
   if(!item) return;

   item->setText(0, prj->title());
}

void
ClientPage::
project_remove(int64_t prj_id)
/***********************************************************
 * A project record was deleted.
 */
{
   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);

   /* If this isn't our project, ignore it */
   if(prj->client_id() != _client.id) return;

   /* See if we can find it in our tree */
   ProjectTreeWidgetItem *item= _project.trw->findItem(prj_id);
   Q_ASSERT(item);

   if(_project.trw->currentItem() == item) {
      _project.page->setProject(-1);
   }

   _project.trw->removeItem(item);
   
}


void
ClientPage::
weeklyReport()
/*****************************************************************************
 * User wants to generate a weekly report.
 */
{
   int rtn;
   /* Run the picker modally */
   WeekChooser wp(this);
   rtn = wp.exec();

   if (rtn == QDialog::Accepted) {
      ClientReport rpt(_client.id, wp.begin(), wp.end());
      ReportWidget *rw= new ReportWidget(rpt);
      rw->show();
   }
}

void
ClientPage::
monthlyReport()
/*****************************************************************************
 * User wants to generate a monthly report.
 */
{
//   J_DBG_FN;
   int rtn;
   /* Run the picker modally */
   MonthChooser mp(this);
   rtn = mp.exec();

   if (rtn == QDialog::Accepted) {
//      J_DBG_FN << "Accepted!";
      ClientReport rpt(_client.id, mp.begin(), mp.end());
      ReportWidget *rw= new ReportWidget(rpt);
      rw->show();
   }
}

void
ClientPage::
customReport()
/*****************************************************************************
 * User wants to generate a custom report.
 */
{
   J_DBG_FN;
   // FIXME: this needs to be implemented
#if 0
   int rtn;
   /* Run the picker modally */
   MonthChooser mp(this);
   rtn = mp.exec();

   if (rtn == QDialog::Accepted) {
//      J_DBG_FN << "Accepted!";
      ClientReport rpt(_client.id, mp.begin(), mp.end());
      ReportWidget *rw= new ReportWidget(rpt);
      rw->show();
   }
#endif
}
