#ifndef REPORT_H
#define REPORT_H

#include <QDateTime>
#include <QWidget>
#include <QMultiHash>
#include <QHash>
#include <QMap>
#include <QStringList>

#include <stdint.h>

class QTextEdit;

/***********************************************************************************/
/*********************** ProjectReport *********************************************/
/***********************************************************************************/
struct ProjectReport {

   int64_t prj_id,
           parent_id;

   int busySecs;

   int run(int64_t project_id, const QDateTime &begin, const QDateTime &end);

};


/***********************************************************************************/
/************************ ClientReport *********************************************/
/***********************************************************************************/
struct ClientReport {

   ClientReport(int64_t client_id, const QDateTime &begin, const QDateTime &end)
   { run(client_id, begin, end);}

   int cumulativeProjectSecs(int64_t prj_id) const;
   int projectSecs(int64_t prj_id) const;

   void activeProjectIds(QMap<QString,ProjectReport*> &prjMap) const;

   int cumulativeSecs() const;

   int64_t client_id;

   QDateTime begin_dt,
             end_dt;
   QMultiHash<int64_t, int64_t> child_hash;
   QHash <int64_t, ProjectReport*> rpt_hash;

   int run(int64_t client_id, const QDateTime &begin, const QDateTime &end);
};

/***********************************************************************************/
/***************************** ReportWidget ****************************************/
/***********************************************************************************/
class ReportWidget : public QWidget {

   Q_OBJECT

public:
   ReportWidget(ClientReport &rpt, QWidget *parent=0);
   ~ReportWidget();

  /* Eliminate default copy constructor and assignment operator */
  ReportWidget(const ReportWidget&)= delete;
  ReportWidget& operator=(const ReportWidget&)= delete;

  int showReport(ClientReport &rpt);

private slots:
   void storeSettings();
   void saveToDisk();

protected:
   void closeEvent(QCloseEvent *event);

private:

  QTextEdit *_te;
  QStringList _html;
};


#endif


