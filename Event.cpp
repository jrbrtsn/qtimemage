#include <QDateTime>

#include "qtimemage.h"
#include "DbRec.h"
#include "Event.h"

const DbField Event::S_DbField_arr[] = {
   { 
      .colName = "id",
      .type = DbField::PKEY_TYPE,
      .colEnum = Event::ID_FIELD,
      .offset = class_offsetof(Event, _id)
   }, {
      .colName = "project_id",
      .type = DbField::INT64_TYPE,
      .colEnum = Event::PROJECT_ID_FIELD,
      .offset = class_offsetof(Event, _project_id)
   }, {
      .colName = "when_ts",
      .type = DbField::INT64_TYPE,
      .colEnum = Event::WHEN_TS_FIELD,
      .offset = class_offsetof(Event, _when_ts)
   }, {
      .colName = "type_enum",
      .type = DbField::INT32_TYPE,
      .colEnum = Event::TYPE_ENUM_FIELD,
      .offset = class_offsetof(Event, _type_enum)
   }, {
      .colName = NULL,
      .type = DbField::UNKOWN_TYPE,
      .colEnum = Event::UNKNOWN_FIELD,
      .offset = 0
   }
};

/****************************************************************************/
/*************************** Event_Table ***********************************/
/****************************************************************************/

Event_Table::
Event_Table(QSqlDatabase &db)
/*****************************************************************
 * Initialize our table
 */
: DbTable(db, "event_tbl", Event::S_DbField_arr)
{
}

Event_Table::
~Event_Table()
/*****************************************************************
 * Free resources
 */
{
}

Event*
Event_Table::
operator[](int64_t id)
/*****************************************************************
 * Access to a common collection of objects.
 */
{
   if(_hash.contains(id)) return _hash[id];
   Event *ev= new Event;
   ev->set_id(id);
   if(ev->fetch(*this)) { /* Problem fetching ? */
      delete ev;
      return NULL;
   }

   _hash[id]= ev;
   return ev;
}

DbRec*
Event_Table::
factory() const
/*****************************************************************
 * Create an instance of Event, return result cast as DbRec*
 */
{
   DbRec *rtn = new Event;
   return rtn;
}

int
Event_Table::
fetchAll(QVector<int64_t> &idVec, const QString & sql_tail)
/*****************************************************************
 * Fetch all Event rows matching sql_tail.
 */
{
   int rtn = EOF - 1;
   unsigned vecSz= 50000;
   DbRec* DbRec_vec[vecSz];
   Event *ev;
   int64_t id;

   if ((rtn = DbTable::fetchAll(DbRec_vec, vecSz, sql_tail)) < 0) goto abort;

   /* Cast pointers back to Event */
   for (int i = 0; i < rtn; ++i) {
      ev= static_cast <Event*>(DbRec_vec[i]);
      id= ev->id();
      idVec.append(id);
      if(_hash.contains(id))
         delete ev;
      else
         _hash[id]= ev;
   }

 abort:
   return rtn;
}

void
Event_Table::
emit_cast_sig_insert(DbRec *dbr)
/*****************************************************************
 * emit sig_insert with the primary key.
 */
{
   /* Get the id from dbr, and get our own copy of the record */
   Event *ev= static_cast<Event*>(dbr);
   int64_t id= ev->id();

   /* Note: the provenance of dbr is unkown, so we have to get our
    * own object.
    */
   ev= new Event;
   ev->set_id(id);
   if(ev->fetch(*this)) Q_ASSERT(0);

   _hash[id]= ev;
   emit sig_insert(id);
}

void
Event_Table::
emit_cast_sig_update(DbRec *dbr)
/*****************************************************************
 * emit sig_update with the primary key.
 */
{
   Event *ev= static_cast<Event*>(dbr);
   int64_t id;
   if(ev->id(&id)) Q_ASSERT(0);
   emit sig_update(id);
}

void
Event_Table::
emit_cast_sig_remove(DbRec *dbr)
/*****************************************************************
 * emit sig_remove with the primary key.
 */
{
   Event *ev= static_cast<Event*>(dbr);
   int64_t id;
   if(ev->id(&id)) Q_ASSERT(0);
   Q_ASSERT(_hash.contains(id));
   emit sig_remove(id);
   _hash.remove(id);
   delete ev;
}

/****************************************************************************/
/************************** Event *****************************************/
/****************************************************************************/

Event::
Event()
/*****************************************************************
 * Default constructor
 */
 : DbRec(this, S_DbField_arr)
{
}

Event::
~Event()
/*****************************************************************
 * Destructor
 */
{
}

void
Event::
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
Event::
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
Event::
set_when_ts(int64_t secs)
/*****************************************************************
 * Set the number of seconds since the Unix epoch for this record.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, WHEN_TS_FIELD);
   _when_ts = secs;
}

void
Event::
set_when_ts(const QDateTime &when)
/*****************************************************************
 * Set the number of seconds since the Unix epoch for this record.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, WHEN_TS_FIELD);
   _when_ts= when.toSecsSinceEpoch();
}

void
Event::
set_type_enum(int32_t type)
/*****************************************************************
 * Set the type_enum for this record.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, TYPE_ENUM_FIELD);
   _type_enum = type;
}


bool
Event::
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
Event::
id() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(ID_FIELD)) qFatal("Field is null");
   return _id;
}

bool
Event::
project_id(int64_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(PROJECT_ID_FIELD)) return true;
   *rtn= _project_id;
   return false;
}

int64_t
Event::
project_id() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(PROJECT_ID_FIELD)) qFatal("Field is null");
   return _project_id;
}

bool
Event::
when_ts(int64_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(WHEN_TS_FIELD)) return true;
   *rtn= _when_ts;
   return false;
}

bool
Event::
when_ts(QDateTime *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(WHEN_TS_FIELD)) return true;
   rtn->setSecsSinceEpoch(_when_ts);
   return false;
}

int64_t
Event::
when_ts() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(WHEN_TS_FIELD)) qFatal("Field is null");
   return _when_ts;
}

bool
Event::
type_enum(int *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(TYPE_ENUM_FIELD)) return true;
   *rtn= _type_enum;
   return false;
}

int
Event::
type_enum() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(TYPE_ENUM_FIELD)) qFatal("Field is null");
   return _type_enum;
}


const char*
Event::
type_str() const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(TYPE_ENUM_FIELD)) qFatal("Field is null");
   switch(_type_enum) {
      case START: return "START";
      case STOP: return "STOP";
      case PAUSE: return "PAUSE";
   }
   Q_ASSERT(0);
}

int
Event::
insert() 
/*****************************************************************
 * Convenience function for this app
 */
{
   return DbRec::insert(G.eventTable);
}

int
Event::
update()
/*****************************************************************
 * Convenience function for this app
 */
{
   return DbRec::update(G.eventTable);
}

int
Event::
remove()
/*****************************************************************
 * Convenience function for this app
 */
{
   return DbRec::remove(G.eventTable);
}

