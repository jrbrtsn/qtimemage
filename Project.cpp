#include <algorithm>
#include <QDateTime>

#include "qtimemage.h"
#include "DbRec.h"
#include "Project.h"

const DbField Project::S_DbField_arr[] = {
   { 
      .colName = "id",
      .type = DbField::PKEY_TYPE,
      .colEnum = Project::ID_FIELD,
      .offset = class_offsetof(Project, _id)
   }, {
      .colName = "client_id",
      .type = DbField::INT64_TYPE,
      .colEnum = Project::CLIENT_ID_FIELD,
      .offset = class_offsetof(Project, _client_id)
   }, {
      .colName = "project_id",
      .type = DbField::INT64_TYPE,
      .colEnum = Project::PROJECT_ID_FIELD,
      .offset = class_offsetof(Project, _project_id)
   }, {
      .colName = "title",
      .type = DbField::STRING_TYPE,
      .colEnum = Project::TITLE_FIELD,
      .offset = class_offsetof(Project, _title)
   }, {
      .colName = "rate",
      .type = DbField::INT64_TYPE,
      .colEnum = Project::RATE_FIELD,
      .offset = class_offsetof(Project, _rate)
   }, {
      .colName = "charge_quantum",
      .type = DbField::INT32_TYPE,
      .colEnum = Project::CHARGE_QUANTUM_FIELD,
      .offset = class_offsetof(Project, _charge_quantum)
   }, {
      .colName = NULL,
      .type = DbField::UNKOWN_TYPE,
      .colEnum = Project::UNKNOWN_FIELD,
      .offset = 0
   }
};

/****************************************************************************/
/*************************** Project_Table ***********************************/
/****************************************************************************/

Project_Table::
Project_Table(QSqlDatabase &db)
/*****************************************************************
 * Initialize our table
 */
: DbTable(db, "project_tbl", Project::S_DbField_arr)
{
}

Project_Table::
~Project_Table()
/*****************************************************************
 * Free resources
 */
{
}

Project*
Project_Table::
operator[](int64_t id)
/*****************************************************************
 * Access to a common collection of objects.
 */
{
   if(_hash.contains(id)) return _hash[id];
   Project *prj= new Project;
   prj->set_id(id);
   if(prj->fetch(*this)) { /* Problem fetching ? */
      delete prj;
      return NULL;
   }

   _hash[id]= prj;
   return prj;
}

DbRec*
Project_Table::
factory() const
/*****************************************************************
 * Create an instance of Project, return result cast as DbRec*
 */
{
   DbRec *rtn = new Project;
   return rtn;
}

int
Project_Table::
fetchClientAfter(QVector<int64_t> &idVec, int64_t client_id, const QDateTime &when)
/****************************************************************************
 * Fetch all Project rows with associated events after when,
 * and matching client_id.
 */
{
   int rtn = EOF - 1;
   unsigned vecSz= 10000;
   Project *prj;
   int64_t id;
   DbRec *DbRec_vec[vecSz];



   QString sql_pfx= QString(
"WITH RECURSIVE tmp00(_id, _project_id) AS ("
" SELECT DISTINCT p.id, p.project_id"
   " FROM"
      " event_tbl ev,"
      " project_tbl p"
   " WHERE ev.when_ts > %1"
   " AND p.id = ev.project_id"
   " AND p.client_id = %2"
" UNION ALL"
" SELECT DISTINCT p.id, p.project_id"
   " FROM"
      " tmp00,"
      " project_tbl p"
   " WHERE p.id = tmp00._project_id )"
            )
      .arg(when.toSecsSinceEpoch())
      .arg(client_id);

   if ((rtn = DbTable::fetchAll_r(DbRec_vec, vecSz, sql_pfx, "tmp00", "_id")) < 0) goto abort;

   /* Cast pointers back to Project */
   for (int i = 0; i < rtn; ++i) {
      prj= static_cast <Project*>(DbRec_vec[i]);
      id= prj->id();
      idVec.append(id);
      if(_hash.contains(id))
         delete prj;
      else
         _hash[id]= prj;
   }

 abort:
   return rtn;
}

int
Project_Table::
fetchAfter(
      QVector<int64_t> &idVec,
      const QDateTime &when
      )
/*****************************************************************
 * Fetch all Project rows with associated events after when,
 and matching sql_tail.
 */
{
   int rtn = EOF - 1;
   unsigned vecSz= 10000;
   Project *prj;
   int64_t id;
   DbRec *DbRec_vec[vecSz];



   QString sql_pfx= QString(
"WITH RECURSIVE tmp00(_id, _project_id) AS ("
" SELECT DISTINCT p.id, p.project_id\n"
   " FROM\n"
      " event_tbl ev,"
      " project_tbl p"
   " WHERE ev.when_ts > %1"
   " AND p.id = ev.project_id"
" UNION ALL"
" SELECT DISTINCT p.id, p.project_id"
   " FROM"
      " tmp00,"
      " project_tbl p"
   " WHERE p.id = tmp00._project_id)"
            ).arg(when.toSecsSinceEpoch());

   if ((rtn = DbTable::fetchAll_r(DbRec_vec, vecSz, sql_pfx, "tmp00", "_id")) < 0) goto abort;

   /* Cast pointers back to Project */
   for (int i = 0; i < rtn; ++i) {
      prj= static_cast <Project*>(DbRec_vec[i]);
      id= prj->id();
      idVec.append(id);
      if(_hash.contains(id))
         delete prj;
      else
         _hash[id]= prj;
   }

 abort:
   return rtn;
}

int
Project_Table::
fetchAll(QVector<int64_t> &idVec, const QString & sql_tail)
/*****************************************************************
 * Fetch all Project rows matching sql_tail.
 */
{
   int rtn = EOF - 1;
   unsigned vecSz= 10000;
   Project *prj;
   int64_t id;
   DbRec *DbRec_vec[vecSz];

   if ((rtn = DbTable::fetchAll(DbRec_vec, vecSz, sql_tail)) < 0) goto abort;

   /* Cast pointers back to Project */
   for (int i = 0; i < rtn; ++i) {
      prj= static_cast <Project*>(DbRec_vec[i]);
      id= prj->id();
      idVec.append(id);
      if(_hash.contains(id))
         delete prj;
      else
         _hash[id]= prj;
   }

 abort:
   return rtn;
}

void
Project_Table::
emit_cast_sig_insert(DbRec *dbr)
/*****************************************************************
 * emit sig_insert with the primary key.
 */
{
//   J_DBG_FN;

   /* Get the id from dbr, and get our own copy of the record */
   Project *prj= static_cast<Project*>(dbr);
   int64_t id= prj->id();

   /* Note: the provenance of dbr is unkown, so we have to get our
    * own object.
    */
   prj= new Project;
   prj->set_id(id);
   if(prj->fetch(*this)) Q_ASSERT(0);


   _hash[id]= prj;
   emit sig_insert(id);
}

void
Project_Table::
emit_cast_sig_update(DbRec *dbr)
/*****************************************************************
 * emit sig_update with the primary key.
 */
{
//   J_DBG_FN;
   Project *prj= static_cast<Project*>(dbr);
   int64_t id= prj->id();
   emit sig_update(id);
}

void
Project_Table::
emit_cast_sig_remove(DbRec *dbr)
/*****************************************************************
 * emit sig_remove with the primary key.
 */
{
//   J_DBG_FN;
   Project *prj= static_cast<Project*>(dbr);
   int64_t id= prj->id();
   Q_ASSERT(_hash.contains(id));
   emit sig_remove(id);
   _hash.remove(id);
   delete prj;
}

/****************************************************************************/
/************************** Project *****************************************/
/****************************************************************************/

Project::
Project()
/*****************************************************************
 * Default constructor
 */
 : DbRec(this, S_DbField_arr)
{
}

Project::
~Project()
/*****************************************************************
 * Destructor
 */
{
}

Event*
Project::
lastEvent() const
/*****************************************************************
 * Retrieve the current event, if one exists.
 */
{

   /* Make sure we have required data */
   if(is_field_null(ID_FIELD)) Q_ASSERT(0);

   QVector<int64_t> event_id_vec;
   int rtn;

   rtn= G.eventTable.fetchAll(event_id_vec, QString("WHERE project_id= %1 ORDER BY when_ts DESC LIMIT 1").arg(_id));

   Q_ASSERT(rtn != -1);

   /* Maybe no events */
   if(!rtn) return NULL;

   int64_t event_id= event_id_vec[0];
   
   return G.eventTable[event_id];
}

enum Project::State
Project::
currentState() const
/*****************************************************************
 * Current state of a project
 */
{
   Event *le= lastEvent();
   if(!le) return STOPPED;

   int type;
   if(le->type_enum(&type)) Q_ASSERT(0);

   switch(type) {

      case Event::START:
         return RECORDING;

      case Event::STOP:
         return STOPPED;

      case Event::PAUSE:
         return PAUSED;
   }

   return ERROR;
}

QIcon
Project::
currentStateIcon() const
/*****************************************************************
 * Return the icon representing the current project state
 */
{
   switch(currentState()) {
      case STOPPED:
         break;
      case RECORDING: return QIcon::fromTheme("emblem-colors-red");
      case PAUSED: return QIcon::fromTheme("media-playback-pause");
      case ERROR: return QIcon::fromTheme("error");
   }

   return QIcon();
}

void
Project::
insertEvent(int eventTypeEnum)
/*****************************************************************
 * Insert a new event record.
 */
{
   /* Make sure we have required data */
   if(is_field_null(ID_FIELD)) Q_ASSERT(0);

   Event ev;

   ev.set_project_id(_id);
   ev.set_when_ts(QDateTime::currentDateTime());
   ev.set_type_enum(eventTypeEnum);

   if(ev.insert()) Q_ASSERT(0);
}

void
Project::
Start()
/*****************************************************************
 * Begin tracking time.
 */
{
   switch(currentState()) {
      case STOPPED:
      case PAUSED:
         insertEvent(Event::START);
         break;

      default:
         Q_ASSERT(0);
   }
}

void
Project::
Stop()
/*****************************************************************
 * Stop tracking time. Minimum charge quantum applies.
 */
{
   switch(currentState()) {
      case RECORDING:
         insertEvent( Event::STOP);
         break;

      case PAUSED:
         { /* In this case, we just change the last event to stop instead of pause */
            Event event, *pLast= lastEvent();
            Q_ASSERT(pLast);
            pLast->set_type_enum(Event::STOP);
            if(pLast->update()) Q_ASSERT(0);
            
         } break;

      default:
         Q_ASSERT(0);
   }
}

void
Project::
Pause()
/*****************************************************************
 * Pause tracking of time.
 */
{
   switch(currentState()) {
      case RECORDING:
         insertEvent(Event::PAUSE);
         break;

      default:
         Q_ASSERT(0);
   }
}

void
Project::
set_id(int64_t id)
/*****************************************************************
 * Set the primary key for this record.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, ID_FIELD);
   _id = id;
}

void
Project::
set_client_id(int64_t id)
/*****************************************************************
 * Set the client_id for this record.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, CLIENT_ID_FIELD);
   _client_id = id;
}

void
Project::
set_project_id(int64_t id)
/*****************************************************************
 * Set the project_id for this record.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, PROJECT_ID_FIELD);
   _project_id = id;
}

void
Project::
set_rate(int64_t rate)
/*****************************************************************
 * Set the rate for this project.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, RATE_FIELD);
   _rate = rate;
}

void
Project::
set_charge_quantum(int32_t charge_quantum)
/*****************************************************************
 * Set the charge quantum for this project
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, CHARGE_QUANTUM_FIELD);
   _charge_quantum = charge_quantum;
}

void
Project::
set_title(const QString & title)
/*****************************************************************
 * Set the title for this client.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, TITLE_FIELD);
   _title = title;
}

bool
Project::
id(int64_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(ID_FIELD)) return true;
   *rtn= _id;
   return false;
}

int64_t
Project::
id() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(ID_FIELD)) qFatal("Field is null");
   return _id;
}

bool
Project::
client_id(int64_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(CLIENT_ID_FIELD)) return true;
   *rtn= _client_id;
   return false;
}

int64_t
Project::
client_id() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(CLIENT_ID_FIELD)) qFatal("Field is null");
   return _client_id;
}

bool
Project::
project_id(int64_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(PROJECT_ID_FIELD)) return true;
   *rtn= _project_id;
   return false;
}

Project*
Project::
parentProject() const
/*****************************************************************
 * Convenience function
 */
{
   if(is_field_null(PROJECT_ID_FIELD)) return NULL;
   return G.projectTable[_project_id];
}

bool
Project::
title(QString *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(TITLE_FIELD)) return true;
   *rtn= _title;
   return false;
}

QString
Project::
title() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(TITLE_FIELD)) qFatal("Field is null");
   return _title;
}

bool
Project::
rate(int64_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(RATE_FIELD)) return true;
   *rtn= _rate;
   return false;
}

int64_t
Project::
rate() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(RATE_FIELD)) qFatal("Field is null");
   return _rate;
}


bool
Project::
charge_quantum(int32_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(CHARGE_QUANTUM_FIELD)) return true;
   *rtn= _charge_quantum;
   return false;
}

int32_t
Project::
charge_quantum() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(CHARGE_QUANTUM_FIELD)) qFatal("Field is null");
   return _charge_quantum;
}


QString
Project::
FQLongTitle() const
/*****************************************************************************
 * Long title with client acronym at beginning.
 */
{
   Client *cl= G.clientTable[client_id()];
   QString acronym= cl->acronym();
   return QString("%1: %2").arg(acronym).arg(longTitle());
}

QString
Project::
longTitle() const
/*****************************************************************************
 * Round up the long title for our this project.
 */
{
   QStringList qsl;
   const Project *prj;

   for(prj= this; prj; prj= G.projectTable[prj->_project_id]) {
      qsl.push_front(prj->title());
      if(prj->is_field_null(PROJECT_ID_FIELD)) break;
   }

   return qsl.join(" / ");
}

int
Project::
reportTime(int64_t begin_secs, int64_t end_secs)
/*****************************************************************************
 * Report how much time was spent as busy from begin_ts to end_ts.
 */
{
   int cumSecs= 0;
   QVector<int64_t> id_vec;
   QString filter= QString("WHERE project_id=%1 AND when_ts > %2 AND when_ts < %3 ORDER BY when_ts ASC")
      .arg(id())
      .arg(begin_secs - PROJECT_REPORT_MARGIN_SECS)
      .arg(end_secs + PROJECT_REPORT_MARGIN_SECS);

   int rtn = G.eventTable.fetchAll(id_vec, filter);
   if (rtn < 0) qFatal("Event::fetchAll() failed!");

   Event *prev_e= NULL;

   for(int i= 0; i < rtn; ++i) {
      Event *e;

      e= G.eventTable[id_vec[i]];
      Q_ASSERT(e);

      switch(e->type_enum()) {

         case Event::STOP:
         case Event::PAUSE:
            if(!i) break;
            if(Event::START != prev_e->type_enum()) return -3600;
            if(e->when_ts() > begin_secs) {
               int period= std::min(e->when_ts(), end_secs) - std::max(prev_e->when_ts(), begin_secs);
               if(e->type_enum() == Event::PAUSE) {
                  cumSecs += period;
               } else {
                  cumSecs += std::max(period, charge_quantum()*60);
               }
            }
            break;

         case Event::START: break;
         default: Q_ASSERT(0);
      }

      /* Bail out if we've passed the end of report period */
      if(e->when_ts() > end_secs) break;

      /* Remember last event */
      prev_e= e;

   }

   /* If the project's charge crosses the end boundary */
   if(prev_e && Event::START == prev_e->type_enum()) {
      cumSecs += end_secs - prev_e->when_ts();
   }

   return cumSecs;
}

int
Project::
insert() 
/*****************************************************************
 * Convenience function for this app
 */
{
   return DbRec::insert(G.projectTable);
}

int
Project::
update()
/*****************************************************************
 * Convenience function for this app
 */
{
   return DbRec::update(G.projectTable);
}

int
Project::
remove()
/*****************************************************************
 * Remove Project record from database, after removing all Event's.
 */
{
   int rtn;
   /* First remove all subprojects */
   QVector<int64_t> subPrj_id_vec;
  
   rtn = G.projectTable.fetchAll(subPrj_id_vec, QString("WHERE project_id=%1").arg(id()));
   if (rtn < 0) qFatal("Project_Table::fetchAll() failed!");

   /* Delete them */
   for (int i = 0; i < subPrj_id_vec.size(); ++i) {
      Project *prj= G.projectTable[subPrj_id_vec[i]];
      Q_ASSERT(prj);
      prj->remove();
   }

   /* Now remove all Event's for this Project */
   QVector<int64_t> event_id_vec;

   rtn = G.eventTable.fetchAll(event_id_vec, QString("WHERE project_id=%1").arg(id()));
   if (rtn < 0) qFatal("EventTable::fetchAll() failed!");

   /* Delete them */
   for (int i = 0; i < event_id_vec.size(); ++i) {
      Event *ev= G.eventTable[event_id_vec[i]];
      Q_ASSERT(ev);
      ev->remove();
   }
  
   return DbRec::remove(G.projectTable);
}

