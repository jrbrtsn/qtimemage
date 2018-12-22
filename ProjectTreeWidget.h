#ifndef PROJECT_TREE_WIDGET
#define PROJECT_TREE_WIDGET

#include <QTreeWidget>
#include <QTreeWidgetItem>

class ClientReport;
class QMenu;
class Project;


/**************************************************************************
 * This wrapper class takes care of the tedium of the generic MVC nonsense,
 * as well as QVariants.
 **************************************************************************/
class ProjectTreeWidgetItem : public QTreeWidgetItem {

public:

   enum {
      TITLE_COL,
      ICON_COL,
      TODAY_COL,
      WEEK_COL,
      MONTH_COL,
      YEAR_COL,
      N_COLS
   };

   ProjectTreeWidgetItem(int64_t prj_id);
   ~ProjectTreeWidgetItem();

  /* Eliminate default copy constructor and assignment operator */
   ProjectTreeWidgetItem(const ProjectTreeWidgetItem&)= delete;
   ProjectTreeWidgetItem& operator=(const ProjectTreeWidgetItem&)= delete;

   ProjectTreeWidgetItem* parent();
   ProjectTreeWidgetItem* child(int ndx);

   int64_t project_id() const;
   void setText(int column, const QString &text);

   void refresh();
};

/**************************************************************************
 * Wrap the QTreeWidget class to customize for our needs.
 **************************************************************************/
class ProjectTreeWidget : public QTreeWidget {

   Q_OBJECT

public:

   ProjectTreeWidget(QWidget *parent);
   ~ProjectTreeWidget();

  /* Eliminate default copy constructor and assignment operator */
   ProjectTreeWidget(const ProjectTreeWidget&)= delete;
   ProjectTreeWidget& operator=(const ProjectTreeWidget&)= delete;

   void setMenu(QMenu *menu);

   ProjectTreeWidgetItem* addProject(int64_t prj_id);
   ProjectTreeWidgetItem* findItem(int64_t prj_id);
   ProjectTreeWidgetItem* currentItem();
   int64_t selectedProject_id();
   int64_t currentProject_id();
   Project* currentProject();
   void removeItem(ProjectTreeWidgetItem *item);

   void refreshTimes(
      const ClientReport &dayRpt,
      const ClientReport &weekRpt,
      const ClientReport &monthRpt,
      const ClientReport &yearRpt
      );

   void setClient(int64_t client_id);

signals:
   void sig_doubleClickedProject(int64_t prj_id);

private slots:
   void showPrjCtxtMenu(const QPoint&);
   void relayDoubleClickedProject(QTreeWidgetItem *item);
   void event_insert(int64_t);
   void event_update(int64_t);
   void event_remove(int64_t);

private:
   void _populate_project(ProjectTreeWidgetItem *parent, QVector<int64_t> &prj_id_vec);
   ProjectTreeWidgetItem* _findItem(int64_t prj_id, ProjectTreeWidgetItem *parent);

   void _refreshTimes(
      ProjectTreeWidgetItem *parent,
      const ClientReport &dayRpt,
      const ClientReport &weekRpt,
      const ClientReport &monthRpt,
      const ClientReport &yearRpt
      );

   QMenu *_menu;
   int64_t _client_id;
};

#endif
