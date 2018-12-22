#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCalendarWidget>
#include <QStatusBar>

#include "util.h"
#include "WeekChooser.h"


WeekChooser::
WeekChooser(QWidget *parent)
/******************************************************************************************
 * Constructor
 */
: QDialog(parent)
{
   setWindowTitle("Week Chooser");
   QVBoxLayout *vbl = new QVBoxLayout(this);

   {  /************************** 1st row ************************/
      QHBoxLayout *hbl = new QHBoxLayout;
      vbl->addLayout(hbl);

      QPushButton *btn= new QPushButton("Today", this);
      hbl->addWidget(btn);

      QLabel *lbl= new QLabel("<b>Selected week:</b>", this);
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

WeekChooser::
~WeekChooser()
/******************************************************************************************
 * Destructor
 */
{
}


void
WeekChooser::
dateChosen(const QDate &when)
/******************************************************************************************
 * A data was chosen
 */
{
   int year, weekNo= when.weekNumber(&year);
   QDateTime end_dt;
   _selection_lbl->setText(QString("Week #%2 %1").arg(year).arg(weekNo));
   _begin_dt.setDate(when.addDays(-when.dayOfWeek()+1));
   end_dt.setDate(_begin_dt.date().addDays(7));
   _end_dt= end_dt.addMSecs(-1);
}

