#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

#include "qtimemage.h"
#include "RateEditor.h"
#include "Event.h"
#include "EventEditor.h"

EventEditor::
EventEditor(int64_t event_id, QWidget *parent)
/*************************************************************************************
 * Construct the dialog.
 */
 : QWidget(parent)
{
   setWindowTitle("Event Editor");

   QVBoxLayout *vbl = new QVBoxLayout(this);

   _tw= new QTreeWidget(this);
   vbl->addWidget(_tw);

#if 0
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
#endif
}

EventEditor::
~EventEditor()
/*************************************************************************************
 * Destruct the dialog.
 */
{
}

