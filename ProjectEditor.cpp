#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

#include "qtimemage.h"
#include "RateEditor.h"
#include "Project.h"
#include "ProjectEditor.h"

ProjectEditor::
ProjectEditor(QWidget *parent, const Project *src)
/*************************************************************************************
 * Construct the dialog.
 */
 : QDialog(parent)
{
   setWindowTitle("Project Editor");

   QVBoxLayout *vbl = new QVBoxLayout(this);

   { /**************** 1st row **********************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      {				/* Title */
	 QLabel *lbl = new QLabel("Title:", this);
	 hbl->addWidget(lbl);
	 _title_le = new QLineEdit(this);
	 hbl->addWidget(_title_le);
         QString title;
         src->title(&title);
	 _title_le->setText(title);
      }

      hbl->addStretch();

      {				/* Rate */
	 QLabel *lbl = new QLabel("Rate:", this);
	 hbl->addWidget(lbl);

         int64_t rate= 0;
         src->rate(&rate);
	 _rate_re = new RateEditor(rate, this);
	 hbl->addWidget(_rate_re);
      }
   }
   
   { /******************* 2nd row ******************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      { /* Billing quantum */
         QLabel *lbl = new QLabel("Billing Quantum (min):", this);
         hbl->addWidget(lbl);
         _charge_quantum_sb= new QSpinBox(this);
         hbl->addWidget(_charge_quantum_sb);
         int32_t charge_quantum=0;
         src->charge_quantum(&charge_quantum);
         _charge_quantum_sb->setValue(charge_quantum);
      }
   }

   { /******************** 3rd row *****************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);
      QDialogButtonBox *dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
      hbl->addWidget(dbb);

      connect(dbb, SIGNAL(rejected()), SLOT(reject()));
      connect(dbb, SIGNAL(accepted()), SLOT(accept()));
   }

}

ProjectEditor::
~ProjectEditor()
/*************************************************************************************
 * Destruct the dialog.
 */
{
}

void
ProjectEditor::
assign(Project *target) const
/*************************************************************************************
 * User has accepted the edit.
 */
{

   /* Put supplied values into the project object */
   target->set_title(_title_le->text());
   target->set_rate(_rate_re->pennies());
   target->set_charge_quantum(_charge_quantum_sb->value());

}
