
#include <assert.h>
#include <QVariant>

#include "configItem.h"
#include "ProjectComboBox.h"
#include "qtimemage.h"
#include "util.h"

//static configIntVec S_project_ids("Project_QuickSelect_projects");
static configInt S_currentProject_id("Project_QuickSelect_current_project", -1);


/*****************************************************************************/
/**************** ProjectComboBox ******************************************/
/*****************************************************************************/

ProjectComboBox::
ProjectComboBox(QWidget *parent)
/*****************************************************************************
 * Constructor
 */
 : QComboBox(parent)
{
   /* Settings management */
   connect(&G, SIGNAL(sig_aboutToSaveSettings()), SLOT(storeSettings()));

   connect(&G.projectTable, SIGNAL(sig_insert(int64_t)), SLOT(project_insert(int64_t)));
   connect(&G.projectTable, SIGNAL(sig_update(int64_t)), SLOT(project_update(int64_t)));
   connect(&G.projectTable, SIGNAL(sig_remove(int64_t)), SLOT(project_remove(int64_t)));

   connect(&G.eventTable, SIGNAL(sig_insert(int64_t)), SLOT(event_insert(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_update(int64_t)), SLOT(event_update(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_remove(int64_t)), SLOT(event_remove(int64_t)));

   setMinimumContentsLength(60);

}

ProjectComboBox::
~ProjectComboBox()
/*****************************************************************************
 * Destructor
 */
{}

void
ProjectComboBox::
project_insert(int64_t prj_id)
/***********************************************************
 * A project record was inserted.
 */
{
   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);
   addProjectSorted(prj);
   int ndx= project_id2ndx(prj_id);
   assert (-1 != ndx);
   setCurrentIndex (ndx);
}

void
ProjectComboBox::
project_update(int64_t prj_id)
/***********************************************************
 * A project record was updated.
 */
{
   /* See if this project is in our combo box */
   int ndx= project_id2ndx(prj_id);
   if(ndx == -1) return;

   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);

   setItemText(ndx, prj->FQLongTitle());
   // TODO: Re-sort?
}

void
ProjectComboBox::
project_remove(int64_t prj_id)
/***********************************************************
 * A project record was removed.
 */
{
   int ndx= project_id2ndx(prj_id);
   if(ndx == -1) return;
   removeItem(ndx);
}

void
ProjectComboBox::
event_insert(int64_t event_id)
/***********************************************************
 * A event record was inserted.
 */
{
   Event *ev= G.eventTable[event_id];
   Q_ASSERT(ev);

   int ndx= project_id2ndx(ev->project_id());
   if(-1 == ndx) return;

   Project *prj= G.projectTable[ev->project_id()];
   Q_ASSERT(prj);

   setItemIcon(ndx, prj->currentStateIcon());

}

void
ProjectComboBox::
event_update(int64_t event_id)
/***********************************************************
 * A event record was
 */
{
   event_insert(event_id);
}

void
ProjectComboBox::
event_remove(int64_t event_id)
/***********************************************************
 * A event record was
 */
{
   event_insert(event_id);
}

void
ProjectComboBox::
storeSettings()
/*****************************************************************************
 * Store the relevant settings
 */
{

   if(currentIndex() != -1) {
      S_currentProject_id= ndx2Project_id(currentIndex());
   }

}

int64_t
ProjectComboBox::
ndx2Project_id(int ndx)
/*****************************************************************************
 * Return the Project id represented by ndx.
 */
{
   QVariant qv= itemData(ndx);
   Q_ASSERT(qv.isValid());
   bool ok;
   int64_t prj_id= qv.toLongLong(&ok);
   Q_ASSERT(ok);
   return prj_id;
}

Project*
ProjectComboBox::
ndx2Project(int ndx)
/*****************************************************************************
 * Return the Project represented by ndx.
 */
{
   return G.projectTable[ndx2Project_id(ndx)];
}

int
ProjectComboBox::
project_id2ndx(int64_t prj_id)
/*****************************************************************************
 * Convert a Project id to a QComboBox index.
 */
{
   QVariant qv= QVariant::fromValue(prj_id);
   return findData(qv);
}

void
ProjectComboBox::
addProjectSorted(Project *prj)
/*****************************************************************************
 * Add a project sorted on FQLongTitle().
 */
{
   QString name= prj->FQLongTitle();
   QVariant qv= QVariant::fromValue(prj->id());

   for(int i= 0; i < count(); ++i) {
      if(itemText(i) < name) continue;
      insertItem(i, prj->currentStateIcon(), name, qv);
      return;
   }

   addItem(prj->currentStateIcon(), name, qv);
}

void
ProjectComboBox::
populate()
/*****************************************************************************
 * Populate the QComboBox with Projects
 */
{

   /* Put all projects into the combo box */
   QVector<int64_t> prj_id_vec;
   int rtn = G.projectTable.fetchAll(prj_id_vec);
   if (rtn < 0) qFatal("Project::fetchAll() failed!");

   for(int i= 0; i < prj_id_vec.size(); ++i) {
      int64_t prj_id= prj_id_vec[i];
      Project *prj= G.projectTable[prj_id];
      addProjectSorted(prj);
   }



   if(S_currentProject_id.val() != -1) {

      int ndx= project_id2ndx(S_currentProject_id.val());
      if(ndx != -1) setCurrentIndex(ndx);
   }
}

void
ProjectComboBox::
removeItem(int ndx)
/*****************************************************************************
 * Overloaded from QComboBox to keep config info current.
 */
{
   storeSettings();
   QComboBox::removeItem(ndx);
}
