#ifndef PROJECTPAGE_H
#define PROJECTPAGE_H

#include <QWidget>

#include "qtimemage.h"

class Project;
class QMenu;
class QLabel;
class QAction;
class QGroupBox;
class QSplitter;
class EventTreeWidget;
class EventTreeWidgetItem;
//class QTreeWidgetItem;
class QShowEvent;

/* Show the working UI for a single project */
class ProjectPage : public QWidget {

   Q_OBJECT

public:

   ProjectPage(QWidget *parent=0);
   ~ProjectPage();

   void setProject(int64_t prj_id);
   Project* project();

private slots:

   void storeSettings();
   void project_update(int64_t prj_id);

protected:

   void showEvent(QShowEvent *event);
   void hideEvent(QHideEvent *event);

private:

   int64_t _project_id;
   QGroupBox *_project_gb;
   QLabel *_rate_lbl,
          *_charge_quantum_lbl;

   QSplitter *_splt;

   EventTreeWidget *_event_trw;

   bool _do_storeSettings;

};


#endif // PROJECTPAGE_H
