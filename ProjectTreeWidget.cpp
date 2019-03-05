#include <QVariant>
#include <QHeaderView>
#include <QMenu>

#include "qtimemage.h"
#include "util.h"
#include "Report.h"
#include "Project.h"
#include "ProjectTreeWidget.h"

/*****************************************************************************/
/**************** ProjectTreeWidgetItem **************************************/
/*****************************************************************************/

ProjectTreeWidgetItem::
ProjectTreeWidgetItem(int64_t prj_id)
/*****************************************************************************
 * Constructor
 */
{
   setData(0, Qt::UserRole, QVariant::fromValue(prj_id));

   for(int col= TODAY_COL; col < N_COLS; ++col) {
      setTextAlignment(col, Qt::AlignRight);
   }

   refresh();
}


ProjectTreeWidgetItem::
~ProjectTreeWidgetItem()
/*****************************************************************************
 * Destructor
 */
{
}

ProjectTreeWidgetItem*
ProjectTreeWidgetItem::
parent()
/*****************************************************************************
 * Return pointer to the parent item, if it exists.
 */
{
   QTreeWidgetItem *item= QTreeWidgetItem::parent();
   return item ? static_cast<ProjectTreeWidgetItem*>(item) : NULL;
}

ProjectTreeWidgetItem*
ProjectTreeWidgetItem::
child(int ndx)
/*****************************************************************************
 * Return child at index cast to our type.
 */
{
   return static_cast<ProjectTreeWidgetItem*>(QTreeWidgetItem::child(ndx));
}

void
ProjectTreeWidgetItem::
setText(int column, const QString &text)
/*****************************************************************************
 * Set the text in a column of this item.
 */
{
   setData(column, Qt::DisplayRole, text);
}

void
ProjectTreeWidgetItem::
refresh()
/*****************************************************************************
 * Update the icon
 */
{
   Project *prj= G.projectTable[project_id()];
   Q_ASSERT(prj);

   setText(TITLE_COL, prj->title());
   setIcon(ICON_COL, prj->currentStateIcon());
}

int64_t
ProjectTreeWidgetItem::
project_id() const
/*****************************************************************************
 * Return the project_id for this item.
 */
{
   QVariant qv= data(0, Qt::UserRole);
   return qv.isValid() ? qv.toLongLong() : -1;
}

/*****************************************************************************/
/**************** ProjectTreeWidget ******************************************/
/*****************************************************************************/

ProjectTreeWidget::
ProjectTreeWidget(QWidget *parent)
/*****************************************************************************
 * Constructor
 */
: QTreeWidget(parent)
, _menu(NULL)
{
   connect(&G.eventTable, SIGNAL(sig_insert(int64_t)), SLOT(event_insert(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_update(int64_t)), SLOT(event_update(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_remove(int64_t)), SLOT(event_remove(int64_t)));

   setUniformRowHeights(true);
   header()->setMinimumSectionSize(10);

   QStringList headers;
   headers << "Projects" << "" << "Today" << "Week" << "Month" << "Year";
   setColumnCount(ProjectTreeWidgetItem::N_COLS);
   setHeaderLabels(headers);
   setAlternatingRowColors(true);
   QTreeWidgetItem *item= headerItem();
   item->setTextAlignment(ProjectTreeWidgetItem::TITLE_COL, Qt::AlignHCenter);
   for(int col= ProjectTreeWidgetItem::TODAY_COL; col < ProjectTreeWidgetItem::N_COLS; ++col) {
      item->setTextAlignment(col, Qt::AlignHCenter);
   }

   connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(relayDoubleClickedProject(QTreeWidgetItem*)));
   connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(relayDoubleClickedProject(QTreeWidgetItem*)));

}

ProjectTreeWidget::
~ProjectTreeWidget()
/*****************************************************************************
 * Destructor
 */
{}

void
ProjectTreeWidget::
setMenu(QMenu *menu)
/*****************************************************************************
 * Use this as the menu.
 */
{
   _menu= menu;

   /* Context menu */
   setContextMenuPolicy(Qt::CustomContextMenu);

   connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showPrjCtxtMenu(const QPoint&)));

}

ProjectTreeWidgetItem*
ProjectTreeWidget::
addProject(int64_t prj_id)
/*****************************************************************************
 * Add a new project entry to the tree.
 */
{
   ProjectTreeWidgetItem *item= new ProjectTreeWidgetItem(prj_id);
   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);
   int64_t prj_project_id;

   /* Case if this project is top level */
   if(prj->project_id(&prj_project_id)) { /* True if project_id is null, therefore top level project */
      addTopLevelItem(item);
   } else {
      /* Otherwise we need to find the parent QTreeWidgetItem */
      ProjectTreeWidgetItem *parent_item;
      if(!(parent_item= findItem(prj_project_id))) Q_ASSERT(0);
      parent_item->addChild(item);
   }
   return item;
}

ProjectTreeWidgetItem*
ProjectTreeWidget::
findItem(int64_t prj_id)
/*****************************************************************************
 * Find an item by it's project_id.
 */
{
   return _findItem(prj_id, static_cast<ProjectTreeWidgetItem*>(invisibleRootItem()));
}

ProjectTreeWidgetItem*
ProjectTreeWidget::
_findItem(int64_t prj_id, ProjectTreeWidgetItem *parent)
/*****************************************************************************
 * Recursive find an item by it's project_id.
 */
{
   ProjectTreeWidgetItem *item, *rtn= NULL;

   /* Search these children, recurse */
   for(int i= 0; i < parent->childCount(); ++i) {
      item= parent->child(i);
      if(prj_id == item->project_id()) return item;
      rtn= _findItem(prj_id, item);
      if(rtn) break;
   }

   return rtn;
}

void
ProjectTreeWidget::
removeItem(ProjectTreeWidgetItem *item)
/*****************************************************************************
 * Remove the item from the tree.
 */
{
   /* Can you believe how complicated this is? */
   ProjectTreeWidgetItem *parent= item->parent();
   int ndx;
   if(parent) {
      ndx= parent->indexOfChild(item);
      Q_ASSERT(ndx != -1);
      delete parent->takeChild(ndx);
   } else {
      ndx= indexOfTopLevelItem(item);
      Q_ASSERT(ndx != -1);
      delete takeTopLevelItem(ndx);
   }
}

ProjectTreeWidgetItem*
ProjectTreeWidget::
currentItem()
/*****************************************************************************
 * Return the current item cast to ProjectTreeWidgetItem, or NULL.
 */
{
   QTreeWidgetItem *item= QTreeWidget::currentItem();
   return item ? static_cast<ProjectTreeWidgetItem*>(item) : NULL;
}

int64_t
ProjectTreeWidget::
currentProject_id()
/*****************************************************************************
 * Return the id of the current project, or -1.
 * Note: it's not clear what "current" means in the context of selectionChanged().
 */
{
   ProjectTreeWidgetItem *item= currentItem();
   return item ? item->project_id() : -1;
}

int64_t
ProjectTreeWidget::
selectedProject_id()
/*****************************************************************************
 * Return the id of the selected project, or -1.
 */
{
   QList<QTreeWidgetItem*> item_lst= selectedItems();
   if(!item_lst.count()) return -1;
   Q_ASSERT(1 == item_lst.count());
   ProjectTreeWidgetItem *item= static_cast<ProjectTreeWidgetItem*>(item_lst[0]);
   return item->project_id();
}

Project*
ProjectTreeWidget::
currentProject()
/*****************************************************************************
 * Return the pointer to the current project, or -1;
 */
{
   int64_t prj_id= currentProject_id();
   return -1 == prj_id ? NULL : G.projectTable[prj_id];
}

void
ProjectTreeWidget::
refreshTimes(
   const ClientReport &dayRpt,
   const ClientReport &weekRpt,
   const ClientReport &monthRpt,
   const ClientReport &yearRpt
)
/******************************************************************************************
 * Function for refreshing project times.
 */
{
   _refreshTimes(
         static_cast<ProjectTreeWidgetItem*>(invisibleRootItem()),
         dayRpt,
         weekRpt,
         monthRpt,
         yearRpt
         );

}

void
ProjectTreeWidget::
_refreshTimes(
      ProjectTreeWidgetItem *parent,
      const ClientReport &dayRpt,
      const ClientReport &weekRpt,
      const ClientReport &monthRpt,
      const ClientReport &yearRpt
      )
/******************************************************************************************
 * Recursive function for refreshing project times.
 */
{
   ProjectTreeWidgetItem *item; 
   int decihours;
   int64_t prj_id;

   /* Search these children, recurse */
   for(int i= 0; i < parent->childCount(); ++i) {
      item= parent->child(i);
      prj_id= item->project_id();

      decihours= dayRpt.cumulativeProjectDecihours(prj_id);
      item->setText(ProjectTreeWidgetItem::TODAY_COL, decihour2hrStr(decihours));

      decihours= weekRpt.cumulativeProjectDecihours(prj_id);
      item->setText(ProjectTreeWidgetItem::WEEK_COL, decihour2hrStr(decihours));

      decihours= monthRpt.cumulativeProjectDecihours(prj_id);
      item->setText(ProjectTreeWidgetItem::MONTH_COL, decihour2hrStr(decihours));

      decihours= yearRpt.cumulativeProjectDecihours(prj_id);
      item->setText(ProjectTreeWidgetItem::YEAR_COL, decihour2hrStr(decihours));

      _refreshTimes(item, dayRpt, weekRpt, monthRpt, yearRpt);
   }
}

void
ProjectTreeWidget::
relayDoubleClickedProject(QTreeWidgetItem *item)
/*****************************************************************************
 * User double clicked on a project.
 */
{
   ProjectTreeWidgetItem *ptwi= static_cast<ProjectTreeWidgetItem*>(item);

   /* Now relay this to subscribers */
   emit sig_doubleClickedProject(ptwi->project_id());
}

void
ProjectTreeWidget::
showPrjCtxtMenu(const QPoint &pnt)
/******************************************************************************************
 * Show the project tree view context menu.
 */
{
   _menu->popup(viewport()->mapToGlobal(pnt));
}

void
ProjectTreeWidget::
_populate_project(ProjectTreeWidgetItem *parent, QVector<int64_t> &prj_id_vec)
/*****************************************************************************
 * Recursive function to get projects populated
 */
{
   ProjectTreeWidgetItem *item;
   Project *prj;
   int64_t prj_id,
           prj_parent_id,
           parent_id= parent ? parent->project_id() : -1;

   for (int i = 0; i < prj_id_vec.size(); ++i) {

      prj_id= prj_id_vec[i];
      prj= G.projectTable[prj_id];

      /* Get relevant info about prj_vec[i] */
      prj_parent_id= -1;
      prj->project_id(&prj_parent_id);

      /* Skip Projects which don't belong here */
      if(parent_id != prj_parent_id) continue;

      item= new ProjectTreeWidgetItem(prj_id);

      if(parent) {
         parent->addChild(item);
      } else {
         addTopLevelItem(item);
      }

      _populate_project(item, prj_id_vec);
   }

}

void
ProjectTreeWidget::
setClient(int64_t client_id)
/*****************************************************************************
 * Populate the tree widget with projects.
 */
{
   /* Make sure no items exist in the QTreeWidget */
   clear();

   _client_id= client_id;

   if(-1 == _client_id) return;

   QVector<int64_t> prj_id_vec;
   int rtn = G.projectTable.fetchAll(prj_id_vec, QString("WHERE client_id=%1").arg(_client_id));
   if (rtn < 0) qFatal("Project::fetchAll() failed!");

   _populate_project(NULL, prj_id_vec);

   expandAll();
}

void
ProjectTreeWidget::
event_insert(int64_t event_id)
/*****************************************************************************
 * A event record was inserted in event_tbl
 */
{
   Event *ev= G.eventTable[event_id];
   Q_ASSERT(ev);

   ProjectTreeWidgetItem *item= findItem(ev->project_id());
   if(!item) return;

   item->refresh();
}

void
ProjectTreeWidget::
event_update(int64_t event_id)
/*****************************************************************************
 * A event record was updated in event_tbl
 */
{
   event_insert(event_id);
}

void
ProjectTreeWidget::
event_remove(int64_t event_id)
/*****************************************************************************
 * A event record was removed in event_tbl
 */
{
   event_insert(event_id);
}
