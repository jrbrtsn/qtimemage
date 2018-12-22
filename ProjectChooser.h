
#ifndef PROJECT_PICKER_H
#define PROJECT_PICKER_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDialog>

class QPushButton;

/**************************************************************************
 * This wrapper class takes care of the tedium of the generic MVC nonsense,
 * as well as QVariants.
 **************************************************************************/
class ProjectChooserTreeWidgetItem : public QTreeWidgetItem {

public:

   enum {
      TITLE_COL,
      ICON_COL,
      N_COLS
   };

   ProjectChooserTreeWidgetItem(int64_t prj_id);
   ~ProjectChooserTreeWidgetItem();

  /* Eliminate default copy constructor and assignment operator */
   ProjectChooserTreeWidgetItem(const ProjectChooserTreeWidgetItem&)= delete;
   ProjectChooserTreeWidgetItem& operator=(const ProjectChooserTreeWidgetItem&)= delete;

   ProjectChooserTreeWidgetItem* parent();
   ProjectChooserTreeWidgetItem* child(int ndx);

   int64_t project_id() const;
   void setText(int column, const QString &text);
};

/**************************************************************************
 * Wrap the QTreeWidget class to customize for our needs.
 **************************************************************************/
class ProjectChooserTreeWidget : public QTreeWidget {

   Q_OBJECT

public:

   ProjectChooserTreeWidget(QWidget *parent);
   ~ProjectChooserTreeWidget();

  /* Eliminate default copy constructor and assignment operator */
   ProjectChooserTreeWidget(const ProjectChooserTreeWidget&)= delete;
   ProjectChooserTreeWidget& operator=(const ProjectChooserTreeWidget&)= delete;

   void setMenu(QMenu *menu);

   int64_t selectedProject_id();

private:
   void _populate_project(ProjectChooserTreeWidgetItem *parent, QVector<int64_t> &prj_id_vec);
   ProjectChooserTreeWidgetItem* _findItem(int64_t prj_id, ProjectChooserTreeWidgetItem *parent);

   int64_t _client_id;
};

/**************************************************************************
 * This is the main class of interest for this header file.
 **************************************************************************/
class ProjectChooser:public QDialog {

 Q_OBJECT
public:
   ProjectChooser(const QString &title, QWidget *parent=0);
   ~ProjectChooser();

   int64_t selectedProject_id();

private slots:

   void selectionChanged();
   void storeSettings();

private:

   QPushButton *_ok_btn;
   ProjectChooserTreeWidget *_trw;
};

#endif
