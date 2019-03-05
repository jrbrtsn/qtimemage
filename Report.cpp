#include <QTextStream>
#include <QMessageBox>
#include <QTextEdit>
#include <QBoxLayout>
#include <QToolBar>
#include <QSizeGrip>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>

#include "qtimemage.h"
#include "util.h"
#include "configItem.h"
#include "Project.h"
#include "Report.h"

static configRect S_geometry("ReportWidget_geom", 20, 20, 400, 800);
static configString S_rptSaveDir("ReportWidget_saveDir");

/***********************************************************************************/
/*********************** ProjectReport *********************************************/
/***********************************************************************************/

int
ProjectReport::
run(int64_t project_id, const QDateTime &begin, const QDateTime &end)
/*************************************************************************
 * Populate data members.
 */
{

   this->prj_id= project_id;
   Project *prj= G.projectTable[project_id];
   Q_ASSERT(prj);

   this->parent_id= -1;
   prj->project_id(&this->parent_id); // May be null

   this->busySecs= prj->reportTime(begin, end);

   return 0;
}

int
ProjectReport::
decihours() const
/*************************************************************************
 * Round busySecs to decihours.
 */
{
   return lround((double)this->busySecs/360.);
}

/***********************************************************************************/
/************************ ClientReport *********************************************/
/***********************************************************************************/
int
ClientReport::
run(int64_t client_id, const QDateTime &begin, const QDateTime &end)
/*************************************************************************
 * Populate data members.
 */
{
   /* Stash a copy for reference later */
   this->begin_dt= begin;
   this->end_dt= end;

   QVector<int64_t> prj_id_vec;
   this->rpt_hash.clear();
   this->child_hash.clear();

   this->client_id= client_id;
   int rtn = G.projectTable.fetchAll(prj_id_vec, QString("WHERE client_id=%1").arg(client_id));
   if (rtn < 0) qFatal("Project::fetchAll() failed!");

   for (int i = 0; i < prj_id_vec.size(); ++i) {
      int64_t prj_id= prj_id_vec[i];
      ProjectReport *pr= new ProjectReport;
      pr->run(prj_id, begin, end);
      this->rpt_hash[prj_id]= pr;

      if(pr->parent_id == -1) continue;

      this->child_hash.insert(pr->parent_id, prj_id);
   }

   return 0;
}

int
ClientReport::
cumulativeDecihours() const
/*************************************************************************
 * Total of all projects for this client.
 */
{
   int rtn= 0;
   ProjectReport *rpt;
   foreach(rpt, this->rpt_hash) {

      rtn += rpt->decihours();
   }
   return rtn;
}

int
ClientReport::
projectDecihours(int64_t prj_id) const
/*************************************************************************
 * Retrieve the decihours for a single project.
 */
{
   const ProjectReport *rpt= this->rpt_hash[prj_id];
   Q_ASSERT(rpt);
   return rpt->decihours();
}

int
ClientReport::
cumulativeProjectDecihours(int64_t prj_id) const
/*************************************************************************
 * Retrieve the cumulative decihours for a project and all of it's subprojects
 */
{
   int rtn= 0;
   const ProjectReport *rpt= this->rpt_hash[prj_id];
   Q_ASSERT(rpt);
   rtn += rpt->decihours();
   QHash<int64_t, int64_t>::const_iterator i = this->child_hash.find(prj_id);
   while (i != this->child_hash.end() && i.key() == prj_id) {
      rtn += cumulativeProjectDecihours(i.value());
      ++i;
   }

   return rtn;
}

void
ClientReport::
activeProjectIds(QMap<QString,ProjectReport*> &prjMap) const
/*************************************************************************
 * Retrieve a vector of all active projects in this report.
 */
{
   Project *prj;
   ProjectReport *pr;
   int64_t prj_id;

   QHash<int64_t, ProjectReport*>::const_iterator i = this->rpt_hash.begin();
   for(; i != this->rpt_hash.end(); ++i) {
      pr= i.value();
      prj_id= pr->prj_id;
      prj= G.projectTable[prj_id];
      Q_ASSERT(prj);
//J_DBG_FN << "prj_id= " << prj_id << ", busySecs= " << pr->busySecs;
      if(!cumulativeProjectDecihours(prj_id)) continue;
      prjMap[prj->longTitle()]= pr;
   }
}

/***********************************************************************************/
/*********************** ReportWidget ****************************************/
/***********************************************************************************/

ReportWidget::
ReportWidget(ClientReport &rpt, QWidget *parent)
/*************************************************************************
 * Constructor
 */
 : QWidget(parent)
{
   /* Listen for Global setVisible signals */
   connect(&G, SIGNAL(sig_setVisibleTopLevelWindows(bool)), SLOT(setVisible(bool)));

   /* Geometry management */
   resize(S_geometry.width(), S_geometry.height());
   connect(&G, SIGNAL(sig_aboutToSaveSettings()), SLOT(storeSettings()));


   QVBoxLayout *vbl= new QVBoxLayout(this);

   { /* tool bar */
      QToolBar *tb= new QToolBar(this);
      vbl->addWidget(tb);

      tb->addAction("Save", this, SLOT(saveToDisk()));
   }

   { /* 2nd row */

      _te= new QTextEdit(this);
      vbl->addWidget(_te);
      _te->setAcceptRichText(true);
      _te->setReadOnly(true);
   }

   { /* Sizegrip */
      QHBoxLayout *hbl= new QHBoxLayout;
      vbl->addLayout(hbl);

      hbl->addStretch(10);

      QSizeGrip *sg= new QSizeGrip(this);
      hbl->addWidget(sg);

   //   vbl->addWidget(sg);
   }
   showReport(rpt);

}


ReportWidget::
~ReportWidget()
/*************************************************************************
 * Destructor
 */
{}

void
ReportWidget::
saveToDisk()
/*************************************************************************
 * Save html to disk.
 */
{
   QString title("Filename for Save");

   for(;;) {
      QString rtn= QFileDialog::getSaveFileName(this, title, S_rptSaveDir);
      if(rtn.isNull()) return;
      QFileInfo fi(rtn);

      S_rptSaveDir= fi.path();
      QFile f(rtn);
      if(!f.open(QFile::WriteOnly|QFile::Truncate)) {
         QString question = QString("Error: Cannot open:\n\"%1\"?").arg(fi.filePath());
         QMessageBox::warning(this,
                              title,
                              question,
                              QMessageBox::Ok);

         continue;
      } else {
         QTextStream strm(&f);
         strm << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n";
         strm << "<html>\n<body>\n";
         strm << _html.join(' ');
         strm << "</body>\n</html>\n";
         break;
      }
   }
}

void
ReportWidget::
storeSettings()
/*************************************************************************
 * Stash geometry
 */
{
  S_geometry= geometry();
}

int
ReportWidget::
showReport(ClientReport &rpt)
/*************************************************************************
 * Generate the text for rpt.
 */
{
   Client *cl= G.clientTable[rpt.client_id];

   setWindowTitle(
         QString("Timecard for %1, %2 - %3")
            .arg(cl->name())
            .arg(rpt.begin_dt.date().toString())
            .arg(rpt.end_dt.date().toString()
         )
       );

   _html.clear();
   _html << QString("<center>\n<h2>Timecard for %1<br>\n%2 - %3</h2></center>\n")
            .arg(cl->name())
            .arg(rpt.begin_dt.date().toString())
            .arg(rpt.end_dt.date().toString());
   _html << "";

   _html << QString("<h3>Cumulative hours, all projects: %1</h3>\n").arg(decihour2hrStr(rpt.cumulativeDecihours()));

   _html << "<table border=1 cellpadding=5>\n";
   _html << "<tr> <th></th> <th colspan=2>Time</th> </tr>\n";
   _html << "<tr> <th>Project</th> <th>Individual</th> <th>Aggregate</th> </tr>\n";
   QMap<QString,ProjectReport*> prjMap;
   rpt.activeProjectIds(prjMap);

   QMap<QString, ProjectReport*>::const_iterator i = prjMap.begin();
   for(; i != prjMap.end(); ++i) {
      ProjectReport *pr= i.value();

      _html << "<tr>"
            << "<td>" << i.key().toHtmlEscaped() << "</td>"
            << "<td align=right>" << decihour2hrStr(pr->decihours()) << "</td>"
            << "<td align=right>" << decihour2hrStr(rpt.cumulativeProjectDecihours(pr->prj_id)) << "</td>"
            << "</tr>\n";

   }

   _html << "</table>\n";

   _te->setText(_html.join(' '));

   return 0;
}

void
ReportWidget::
closeEvent(QCloseEvent *event)
/*************************************************************************
 * Reimplemented so we can save the geometry.
 */
{
   storeSettings();
   deleteLater();
}
