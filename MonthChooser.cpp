#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QStatusBar>
#include <QCalendarWidget>

#include "util.h"
#include "MonthChooser.h"


MonthChooser::
MonthChooser(QWidget *parent)
/******************************************************************************************
 * Constructor
 */
: QDialog(parent)
{
   setWindowTitle("Month Chooser");
   QVBoxLayout *vbl = new QVBoxLayout(this);

   {  /************************** 1st row ************************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      QPushButton *btn= new QPushButton("Today", this);
      hbl->addWidget(btn);

      QLabel *lbl= new QLabel("<b>Selected month:</b>", this);
      hbl->addWidget(lbl);

      _selection_lbl= new QLabel(this);
      hbl->addWidget(_selection_lbl);

      hbl->addStretch(10);
   }

   {  /************************** 2nd row ************************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      _cw= new QCalendarWidget(this);
      hbl->addWidget(_cw);

      connect(_cw, SIGNAL(activated(const QDate&)), SLOT(dateChosen(const QDate&)));
      connect(_cw, SIGNAL(clicked(const QDate&)), SLOT(dateChosen(const QDate&)));

      hbl->addStretch();
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

   QStatusBar *sb= new QStatusBar(this);
   vbl->addWidget(sb);

   dateChosen(QDate::currentDate());
}

MonthChooser::
~MonthChooser()
/******************************************************************************************
 * Destructor
 */
{
}


void
MonthChooser::
dateChosen(const QDate &when)
/******************************************************************************************
 * A data was chosen
 */
{
   QDateTime end_dt;
   _selection_lbl->setText(when.toString("MMMM yyyy"));
   _begin_dt.setDate(when.addDays(-when.day()+1));
   end_dt.setDate(_begin_dt.date().addMonths(1));
   _end_dt= end_dt.addMSecs(-1);
}

