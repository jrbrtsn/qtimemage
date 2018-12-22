#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

#include "Client.h"
#include "RateEditor.h"
#include "ClientEditor.h"

ClientEditor::
ClientEditor(QWidget *parent, const Client *src)
/*************************************************************************************
 * Construct the dialog.
 */
 : QDialog(parent)
{
   setWindowTitle("Client Editor");

   QVBoxLayout *vbl = new QVBoxLayout(this);

   {  /************************** 1st row ************************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      {				/* Acronym */
	 QLabel *lbl = new QLabel("Acronym:", this);
	 hbl->addWidget(lbl);
	 _acronym_le = new QLineEdit(this);
	 hbl->addWidget(_acronym_le);
         QString acronym;
         src->acronym(&acronym);
	 _acronym_le->setText(acronym);
      }

      hbl->addStretch();

      {				/* Default Rate */
	 QLabel *lbl = new QLabel("Default Rate: $", this);
	 hbl->addWidget(lbl);

         int64_t default_rate=0;
         src->default_rate(&default_rate);

	 _default_rate_re = new RateEditor(default_rate, this);
	 hbl->addWidget(_default_rate_re);

      }

   }

   {  /************************** 2nd row ************************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      { /* Full name */
         QLabel *lbl = new QLabel("Full Name:", this);
         hbl->addWidget(lbl);
         _name_le = new QLineEdit(this);
         hbl->addWidget(_name_le);
         QString name;
         src->name(&name);
         _name_le->setText(name);
      }

      { /* Charge quantum */
         QLabel *lbl = new QLabel("Billing Quantum (min):", this);
         hbl->addWidget(lbl);
         _charge_quantum_sb= new QSpinBox(this);
         hbl->addWidget(_charge_quantum_sb);
         int32_t charge_quantum=10;
         src->default_charge_quantum(&charge_quantum);
         _charge_quantum_sb->setValue(charge_quantum);
      }
   }

   {				/* Accept & Cancel buttons */
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);
      QDialogButtonBox *dbb =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
      hbl->addWidget(dbb);

      connect(dbb, SIGNAL(rejected()), SLOT(reject()));
      connect(dbb, SIGNAL(accepted()), SLOT(accept()));
   }

}

ClientEditor::
~ClientEditor()
/*************************************************************************************
 * Destruct the dialog.
 */
{
}

void
ClientEditor::
assign(Client *target) const
/*************************************************************************************
 * Assign editor values to the target
 */
{
   /* Put supplied values into the client object */
   target->set_acronym(_acronym_le->text());
   target->set_name(_name_le->text());
   target->set_default_rate(_default_rate_re->pennies());
   target->set_default_charge_quantum(_charge_quantum_sb->value());
}
