#ifndef CLIENTPAGE_H
#define CLIENTPAGE_H

#include <QWidget>
#include <QVector>
#include "qtimemage.h"

class ProjectTreeWidget;
class QTreeWidgetItem;
class QGroupBox;
class Client;
class Project;
class ProjectPage;
class QMenu;
class QAction;
class QLabel;
class QSplitter;
class QShowEvent;
class QTimer;
class ClientReport;


/* Show the working UI for a single client */
class ClientPage : public QWidget {

   Q_OBJECT

public:
   ClientPage(QWidget *parent=0);

   ~ClientPage();

   void setClient(int64_t client_id, int64_t currentProject_id);

   int64_t client_id() const {return _client.id;}

   int64_t currentProject_id();

signals:

   void sig_doubleClickedProject(int64_t prj_id);
   void sig_projectSelected(int64_t client_id, int64_t project_id);

public slots:

   void showProject(int64_t prj_id);
   void setBillingProject(int64_t prj_id);

private slots:

   void refreshTimes();
   void setCurrentItem();
   void populate_projects();
   void storeSettings();
   void createProject();
   void createSubproject();
   void editProject();
   void deleteProject();
   void projectSelectionChanged();
   void weeklyReport();
   void monthlyReport();
   void customReport();

   void client_update(int64_t);

   void project_insert(int64_t);
   void project_update(int64_t);
   void project_remove(int64_t);

#if 0
  void event_insert(int64_t);
  void event_update(int64_t);
  void event_remove(int64_t);
#endif


protected:

   void showEvent(QShowEvent *event);
   void hideEvent(QHideEvent *event);

private:

   void _refreshTimes(
         QTreeWidgetItem *parent,
         const ClientReport *dayRpt,
         const ClientReport *weekRpt,
         const ClientReport *monthRpt,
         const ClientReport *yearRpt
         );

   QTimer *_refreshTimer;
   void _populate_project(QTreeWidgetItem *parent, QVector<int64_t> &prj_id_vec);
   QTreeWidgetItem* findItem(int64_t prj_id, QTreeWidgetItem *parent=0);

   struct {
      int64_t id;
      QGroupBox *gb;
      QLabel *rate_lbl,
             *charge_quantum_lbl,
             *timeToday_lbl,
             *timeWeek_lbl,
             *timeMonth_lbl,
             *timeYear_lbl;

      struct {
         QAction *weekly_act,
                 *monthly_act,
                 *custom_act;
         QMenu *menu;
      } report;
   } _client;

   QSplitter *_splt;

   struct {
      ProjectPage *page;
      ProjectTreeWidget *trw;
      QAction *create_act,
              *createSubproject_act,
              *edit_act,
              *delete_act;
      QMenu *menu;
   } _project;

   bool _do_storeSettings;

};

#endif // CLIENTPAGE_H
