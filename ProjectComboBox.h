
#ifndef PROJECT_COMBO_BOX
#define PROJECT_COMBO_BOX

#include <QComboBox>

class Project;


/**************************************************************************
 * Wrap the QComboBox class to customize for our needs.
 **************************************************************************/
class ProjectComboBox : public QComboBox {

   Q_OBJECT

public:

   ProjectComboBox(QWidget *parent);
   ~ProjectComboBox();

  /* Eliminate default copy constructor and assignment operator */
   ProjectComboBox(const ProjectComboBox&)= delete;
   ProjectComboBox& operator=(const ProjectComboBox&)= delete;

   void addProjectSorted(Project *prj);
   int64_t ndx2Project_id(int ndx);
   Project* ndx2Project(int ndx);
   void populate();
   int project_id2ndx(int64_t prj_id);
   void removeItem(int ndx);

private slots:

   void storeSettings();

   void project_insert(int64_t);
   void project_update(int64_t);
   void project_remove(int64_t);

   void event_insert(int64_t);
   void event_update(int64_t);
   void event_remove(int64_t);

private:


};

#endif
