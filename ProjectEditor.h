#ifndef PROJECT_EDITOR_H
#define PROJECT_EDITOR_H

#include <QDialog>

class QLineEdit;
class RateEditor;
class QSpinBox;
class Project;

class ProjectEditor:public QDialog {

 Q_OBJECT
public:
   ProjectEditor(QWidget *parent, const Project *src);
   ~ProjectEditor();

   void assign(Project *target) const;

 private:
   QLineEdit *_title_le;
   RateEditor *_rate_re;
   QSpinBox *_charge_quantum_sb;
};

#endif				// PROJECT_EDITOR_H
