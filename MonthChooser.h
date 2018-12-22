#ifndef MONTH_PICKER_H
#define MONTH_PICKER_H

#include <QDialog>
#include <QDateTime>

class QCalendarWidget;
class QLabel;

class MonthChooser:public QDialog {

 Q_OBJECT
public:
   MonthChooser(QWidget *parent);
   ~MonthChooser();

   const QDateTime& begin() {return _begin_dt;}
   const QDateTime& end() {return _end_dt;}

private slots:

   void dateChosen(const QDate &when);

private:
   QCalendarWidget *_cw;
   QDateTime _begin_dt,
             _end_dt;
   QLabel *_selection_lbl;
};




#endif
