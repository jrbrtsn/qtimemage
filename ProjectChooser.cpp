#include <QBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLabel>
#include <QStatusBar>
#include <QPushButton>
#include <QDialogButtonBox>

#include "util.h"
#include "configItem.h"
#include "qtimemage.h"
#include "Project.h"
#include "Client.h"
#include "ProjectChooser.h"

static configRect S_geometry("ProjectChooser_geom", 20, 20, 600, 400);
static configIntVec S_projectHdrSettings("ProjectChooser_ProjectsTree_header_settings");

/*****************************************************************************/
/**************** ProjectChooserTreeWidgetItem **************************************/
/*****************************************************************************/

ProjectChooserTreeWidgetItem::
ProjectChooserTreeWidgetItem(int64_t prj_id)
/*****************************************************************************
 * Constructor
 */
{
   Project *prj= G.projectTable[prj_id];
   Q_ASSERT(prj);

   setText(TITLE_COL, prj->FQLongTitle());
   setData(0, Qt::UserRole, QVariant::fromValue(prj_id));

}


ProjectChooserTreeWidgetItem::
~ProjectChooserTreeWidgetItem()
/*****************************************************************************
 * Destructor
 */
{
}

ProjectChooserTreeWidgetItem*
ProjectChooserTreeWidgetItem::
parent()
/*****************************************************************************
 * Return pointer to the parent item, if it exists.
 */
{
   QTreeWidgetItem *item= QTreeWidgetItem::parent();
   return item ? static_cast<ProjectChooserTreeWidgetItem*>(item) : NULL;
}

ProjectChooserTreeWidgetItem*
ProjectChooserTreeWidgetItem::
child(int ndx)
/*****************************************************************************
 * Return child at index cast to our type.
 */
{
   return static_cast<ProjectChooserTreeWidgetItem*>(QTreeWidgetItem::child(ndx));
}

void
ProjectChooserTreeWidgetItem::
setText(int column, const QString &text)
/*****************************************************************************
 * Set the text in a column of this item.
 */
{
   setData(column, Qt::DisplayRole, text);
}

int64_t
ProjectChooserTreeWidgetItem::
project_id() const
/*****************************************************************************
 * Return the project_id for this item.
 */
{
   QVariant qv= data(0, Qt::UserRole);
   return qv.isValid() ? qv.toLongLong() : -1;
}

/*****************************************************************************/
/**************** ProjectChooserTreeWidget ***********************************/
/*****************************************************************************/

ProjectChooserTreeWidget::
ProjectChooserTreeWidget(QWidget *parent)
/*****************************************************************************
 * Constructor
 */
: QTreeWidget(parent)
{
   setUniformRowHeights(true);
   setRootIsDecorated(false);
   header()->setMinimumSectionSize(10);

   QStringList headers;
   headers << "Project" << "";
   setColumnCount(ProjectChooserTreeWidgetItem::N_COLS);
   setHeaderLabels(headers);
   setAlternatingRowColors(true);
   QTreeWidgetItem *item= headerItem();
   item->setTextAlignment(ProjectChooserTreeWidgetItem::TITLE_COL, Qt::AlignHCenter);

   QVector<int64_t> prj_id_vec;
   int rtn = G.projectTable.fetchAll(prj_id_vec);
   if (rtn < 0) qFatal("Project::fetchAll() failed!");

   _populate_project(NULL, prj_id_vec);

   sortItems(ProjectChooserTreeWidgetItem::TITLE_COL, Qt::AscendingOrder);

   expandAll();

}

ProjectChooserTreeWidget::
~ProjectChooserTreeWidget()
/*****************************************************************************
 * Destructor
 */
{}


int64_t
ProjectChooserTreeWidget::
selectedProject_id()
/*****************************************************************************
 * Return the id of the selected project, or -1.
 */
{
   QList<QTreeWidgetItem*> item_lst= selectedItems();
   if(!item_lst.count()) return -1;
   Q_ASSERT(1 == item_lst.count());
   ProjectChooserTreeWidgetItem *item= static_cast<ProjectChooserTreeWidgetItem*>(item_lst[0]);
   return item->project_id();
}

#if 0
void
ProjectChooserTreeWidget::
relayDoubleClickedProject(QTreeWidgetItem *item)
/*****************************************************************************
 * User double clicked on a project.
 */
{
   ProjectChooserTreeWidgetItem *ptwi= static_cast<ProjectChooserTreeWidgetItem*>(item);

   /* Now relay this to subscribers */
   emit sig_doubleClickedProject(ptwi->project_id());
}
#endif

void
ProjectChooserTreeWidget::
_populate_project(ProjectChooserTreeWidgetItem *parent, QVector<int64_t> &prj_id_vec)
/*****************************************************************************
 * Recursive function to get projects populated
 */
{
   ProjectChooserTreeWidgetItem *item;
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

      item= new ProjectChooserTreeWidgetItem(prj_id);

      /* Show the title in the tree view */
      item->setIcon(ProjectChooserTreeWidgetItem::ICON_COL, prj->currentStateIcon());

      if(parent) {
         parent->addChild(item);
      } else {
         addTopLevelItem(item);
      }

      _populate_project(item, prj_id_vec);
   }

}

/*****************************************************************************/
/**************** ProjectChooser *********************************************/
/*****************************************************************************/

ProjectChooser::
ProjectChooser(const QString &title, QWidget *parent)
/******************************************************************************************
 * Constructor
 */
: QDialog(parent)
{
   setWindowTitle("Project Chooser");
   setGeometry(S_geometry);

   connect(this, SIGNAL(accepted()), SLOT(storeSettings()));
   connect(this, SIGNAL(rejected()), SLOT(storeSettings()));


   QVBoxLayout *vbl = new QVBoxLayout(this);

   {  /************************** 1st row ************************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      QLabel *lbl= new QLabel(QString("<b>%1</b>").arg(title), this);
      hbl->addWidget(lbl);

   }

   {  /************************** 2nd row ************************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      _trw= new ProjectChooserTreeWidget(this);
      hbl->addWidget(_trw);

      connect(_trw, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));
      connect(_trw, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(accept()));

      /* Project QTreeWidget settings */
      for(int i= 0; i < _trw->columnCount() && i < S_projectHdrSettings.count(); ++i) {
        _trw->setColumnWidth(i, S_projectHdrSettings[i]);
      }

   }

   {				/* Accept & Cancel buttons */
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);
      QDialogButtonBox *dbb= new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
      hbl->addWidget(dbb);

      _ok_btn= dbb->button(QDialogButtonBox::Ok);
      _ok_btn->setEnabled(false);

      connect(dbb, SIGNAL(rejected()), SLOT(reject()));
      connect(dbb, SIGNAL(accepted()), SLOT(accept()));
   }

   QStatusBar *sb= new QStatusBar(this);
   vbl->addWidget(sb);

}

ProjectChooser::
~ProjectChooser()
/******************************************************************************************
 * Destructor
 */
{
}

int64_t
ProjectChooser::
selectedProject_id()
/******************************************************************************************
 * Which project was selected.
 */
{
   return _trw->selectedProject_id();
}

void
ProjectChooser::
selectionChanged()
/******************************************************************************************
 * The selection has changed.
 */
{
   if(_trw->selectedItems().count()) {
      _ok_btn->setEnabled(true);
   } else {
      _ok_btn->setEnabled(false);
   }
}

void
ProjectChooser::
storeSettings()
/*****************************************************************************
 * Store settings if we should.
 */
{
  S_geometry= geometry();

   /* project QTreeWidget */
   S_projectHdrSettings.resize(_trw->columnCount());
   for(int i= 0; i < _trw->columnCount(); ++i) {
      S_projectHdrSettings[i]= _trw->columnWidth(i);
   }
}

