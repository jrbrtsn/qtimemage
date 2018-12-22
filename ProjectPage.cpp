
#include <QShowEvent>
#include <QDateTime>
#include <QMenu>
#include <QBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QToolButton>
#include <QMessageBox>
#include <QSplitter>

#include "qtimemage.h"
#include "EventTreeWidget.h"
#include "ProjectEditor.h"
#include "Project.h"
#include "configItem.h"
#include "util.h"
#include "ProjectPage.h"

static configIntVec S_eventHdrSettings("ProjectPage_EventsList_header_settings");

ProjectPage::
ProjectPage(QWidget *parent)
/******************************************************************************************
 * Constructor
 */
: QWidget(parent),
_project_id(-1),
_do_storeSettings(false)
{
   /* Geometry management */
   connect(&G, SIGNAL(sig_aboutToSaveSettings()), SLOT(storeSettings()));

   /* Record notifications */
   connect(&G.projectTable, SIGNAL(sig_update(int64_t)), SLOT(project_update(int64_t)));

   QVBoxLayout *vbl= new QVBoxLayout(this);

   _splt= new QSplitter(this);
   _splt->setOrientation(Qt::Vertical);
   vbl->addWidget(_splt);

   _project_gb= new QGroupBox(this);
   _project_gb->setAlignment(Qt::AlignHCenter);
   _splt->addWidget(_project_gb);
   
   { /* Top row */
      QLabel *lbl;
      QVBoxLayout *vbl= new QVBoxLayout(_project_gb);
      _project_gb->setLayout(vbl);

      QHBoxLayout *hbl= new QHBoxLayout;
      vbl->addLayout(hbl);


      hbl->addStretch(10);

      lbl= new QLabel("<b>Rate:</b> $", _project_gb);
      hbl->addWidget(lbl);
      _rate_lbl= new QLabel(_project_gb);
      hbl->addWidget(_rate_lbl);

      hbl->addStretch(10);

      /* Default charge quantum for client */
      lbl= new QLabel("<b>Min. Billing (min):</b>", _project_gb);
      hbl->addWidget(lbl);
      _charge_quantum_lbl= new QLabel(_project_gb);
      hbl->addWidget(_charge_quantum_lbl);

      hbl->addStretch(10);

   }

   { /* Events */
      _event_trw= new EventTreeWidget(_splt);
      _splt->addWidget(_event_trw);
   }

//   vbl->addStretch(10);

}

ProjectPage::
~ProjectPage()
/******************************************************************************************
 * Destructor
 */
{
}

void
ProjectPage::
showEvent(QShowEvent *event)
/*******************************************************************************************
 * Reimplementation of QWidget function.
 */
{
   if(event->type() == QEvent::Show) {
      for(int i= 0; i < _event_trw->columnCount() && i < S_eventHdrSettings.count(); ++i) {
        _event_trw->setColumnWidth(i, S_eventHdrSettings[i]);
      }
      _do_storeSettings= true;
   }

   /* Chain to parent's showEvent() */
   QWidget::showEvent(event);
}

void
ProjectPage::
hideEvent(QHideEvent * event)
/*******************************************************************************************
 * Reimplementation of QWidget function.
 */
{
   if(event->type() == QEvent::Hide) {
      storeSettings();
      _do_storeSettings= false;
   }

   /* Call parent's function */
   QWidget::hideEvent(event);
}

void
ProjectPage::
storeSettings()
/*****************************************************************************
 * Store settings if we should.
 */
{
   if(!_do_storeSettings) return;

   S_eventHdrSettings.resize(_event_trw->columnCount());
   for(int i= 0; i < _event_trw->columnCount(); ++i) {
      S_eventHdrSettings[i]= _event_trw->columnWidth(i);
   }
}

Project*
ProjectPage::
project()
/*****************************************************************************
 * Return our project, or NULL.
 */
{
   if(_project_id == -1) return NULL;
   return G.projectTable[_project_id];
}

void
ProjectPage::
setProject(int64_t prj_id)
/*****************************************************************************
 * Show data for a different project, or none.
 */
{
//   _event_trw->clear();
   _project_id= prj_id;

   if(-1 == prj_id) {

      _event_trw->setProject(-1);

      _project_gb->setTitle(">No Project<");
      _rate_lbl->setText(QString("N/A"));

   } else {

      _event_trw->setProject(prj_id);
      project_update(prj_id);
   }
}

void
ProjectPage::
project_update(int64_t prj_id)
/*****************************************************************************
 * G.clientTable signalled that a record was updated.
 */
{
   /* If this isn't our project, ignore it */
   if(prj_id != _project_id) return;

   if(_project_id != -1) {
      Project *prj= G.projectTable[_project_id];
      Q_ASSERT(prj);

      _project_gb->setTitle(prj->longTitle());

      _rate_lbl->setText(cent2dollarStr(prj->rate()));

      _charge_quantum_lbl->setText(QString("%1").arg(prj->charge_quantum()));

   } else {
      _rate_lbl->setText("N/A");
      _charge_quantum_lbl->setText("N/A");
   }

}

