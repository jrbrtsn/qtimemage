#include "qtimemage.h"
#include "DbRec.h"
#include "Client.h"

const DbField Client::S_DbField_arr[] = {
   { 
      .colName = "id",
      .type = DbField::PKEY_TYPE,
      .colEnum = Client::ID_FIELD,
      .offset = class_offsetof(Client, _id)
   }, {
      .colName = "acronym",
      .type = DbField::STRING_TYPE,
      .colEnum = Client::ACRONYM_FIELD,
      .offset = class_offsetof(Client, _acronym)
   }, {
      .colName = "name",
      .type = DbField::STRING_TYPE,
      .colEnum = Client::NAME_FIELD,
      .offset = class_offsetof(Client, _name)
   }, {
      .colName = "default_rate",
      .type = DbField::INT64_TYPE,
      .colEnum = Client::DEFAULT_RATE_FIELD,
      .offset = class_offsetof(Client, _default_rate)
   }, {
      .colName = "default_charge_quantum",
      .type = DbField::INT32_TYPE,
      .colEnum = Client::DEFAULT_CHARGE_QUANTUM_FIELD,
      .offset = class_offsetof(Client, _default_charge_quantum)
   }, {
      .colName = NULL,
      .type = DbField::UNKOWN_TYPE,
      .colEnum = Client::UNKNOWN_FIELD,
      .offset = 0
   }
};

/****************************************************************************/
/*************************** Client_Table ***********************************/
/****************************************************************************/

Client_Table::
Client_Table(QSqlDatabase &db)
/*****************************************************************
 * Initialize our table
 */
: DbTable(db, "client_tbl", Client::S_DbField_arr)
{
}

Client_Table::
~Client_Table()
/*****************************************************************
 * Free resources
 */
{
}

Client*
Client_Table::
operator[](int64_t id)
/*****************************************************************
 * Access to a common collection of objects.
 */
{
   if(_hash.contains(id)) return _hash[id];
   Client *cl= new Client;
   cl->set_id(id);
   if(cl->fetch(*this)) { /* Problem fetching ? */
      delete cl;
      return NULL;
   }

   _hash[id]= cl;
   return cl;
}

DbRec*
Client_Table::
factory() const
/*****************************************************************
 * Create an instance of Client, return result cast as DbRec*
 */
{
   DbRec *rtn = new Client;
   return rtn;
}

int
Client_Table::
fetchAll(QVector<int64_t> &idVec, const QString & sql_tail)
/*****************************************************************
 * Fetch all Client rows matching sql_tail.
 */
{
   int rtn = EOF - 1;
   unsigned vecSz= 1000;
   DbRec *DbRec_vec[vecSz];
   Client *cl;
   int64_t id;

   if ((rtn = DbTable::fetchAll(DbRec_vec, vecSz, sql_tail)) < 0) goto abort;

   /* Cast pointers back to Client */
   for (int i = 0; i < rtn; ++i) {
      cl= static_cast <Client*>(DbRec_vec[i]);
      id= cl->id();
      idVec.append(id);
      if(_hash.contains(id))
         delete cl;
      else
         _hash[id]= cl;
   }

 abort:
   return rtn;
}

int
Client_Table::
fetchAfter(
      QVector<int64_t> &idVec,
      const QDateTime &when
      )
/*****************************************************************
 * Fetch all Client rows with associated events after when.
 */
{
   int rtn = EOF - 1;
   unsigned vecSz= 1000;
   Client *cl;
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
" UNION ALL"
" SELECT DISTINCT p.id, p.project_id"
   " FROM"
      " tmp00,"
      " project_tbl p"
   " WHERE p.id = tmp00._project_id )"

", tmp01(_id) AS ("
" SELECT DISTINCT c.id"
   " FROM"
      " tmp00,"
      " project_tbl p,"
      " client_tbl c"
   " WHERE p.id = tmp00._id"
   " AND c.id = p.client_id )"
      )
      .arg(when.toSecsSinceEpoch());

   if ((rtn = DbTable::fetchAll_r(DbRec_vec, vecSz, sql_pfx, "tmp01", "_id")) < 0) goto abort;

   /* Cast pointers back to Client */
   for (int i = 0; i < rtn; ++i) {
      cl= static_cast <Client*>(DbRec_vec[i]);
      id= cl->id();
      idVec.append(id);
      if(_hash.contains(id))
         delete cl;
      else
         _hash[id]= cl;
   }

 abort:
   return rtn;
}

void
Client_Table::
emit_cast_sig_insert(DbRec *dbr)
/*****************************************************************
 * emit sig_insert with the primary key.
 */
{
   /* Get the id from dbr, and get our own copy of the record */
   Client *cl= static_cast<Client*>(dbr);
   int64_t id= cl->id();

   /* Note: the provenance of dbr is unkown, so we have to get our
    * own object.
    */
   cl= new Client;
   cl->set_id(id);
   if(cl->fetch(*this)) Q_ASSERT(0);

   _hash[id]= cl;
   emit sig_insert(id);
}

void
Client_Table::
emit_cast_sig_update(DbRec *dbr)
/*****************************************************************
 * emit sig_update with the primary key.
 */
{
   Client *cl= static_cast<Client*>(dbr);
   int64_t id= cl->id();
   emit sig_update(id);
}

void
Client_Table::
emit_cast_sig_remove(DbRec *dbr)
/*****************************************************************
 * emit sig_remove with the primary key.
 */
{
   Client *cl= static_cast<Client*>(dbr);
   int64_t id= cl->id();
   Q_ASSERT(_hash.contains(id));
   emit sig_remove(id);
   _hash.remove(id);
   delete cl;
}

/****************************************************************************/
/**************************** Client ****************************************/
/****************************************************************************/
Client::
Client()
/*****************************************************************
 * Default constructor
 */
 : DbRec(this, S_DbField_arr)
{
}

Client::
~Client()
/*****************************************************************
 * Destructor
 */
{
}

void
Client::
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
Client::
set_default_rate(int64_t default_rate)
/*****************************************************************
 * Set the default rate for this client.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, DEFAULT_RATE_FIELD);
   _default_rate = default_rate;
}

void
Client::
set_default_charge_quantum(int32_t default_charge_quantum)
/*****************************************************************
 * Set the default rate for this client.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, DEFAULT_CHARGE_QUANTUM_FIELD);
   _default_charge_quantum = default_charge_quantum;
}

void
Client::
set_acronym(const QString & acronym)
/*****************************************************************
 * Set the acronym for this client.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, ACRONYM_FIELD);
   _acronym = acronym;
}

void
Client::
set_name(const QString & name)
/*****************************************************************
 * Set the name for this client.
 */
{
   /* Clear the null bit for this field */
   set_field_null(false, NAME_FIELD);
   _name = name;
}

bool
Client::
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
Client::
id() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(ID_FIELD)) qFatal("Field is null");
   return _id;
}

bool
Client::
acronym(QString *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(ACRONYM_FIELD)) return true;
   *rtn= _acronym;
   return false;
}

QString
Client::
acronym() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(ACRONYM_FIELD)) qFatal("Field is null");
   return _acronym;
}

bool
Client::
name(QString *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(ACRONYM_FIELD)) return true;
   *rtn= _name;
   return false;
}

QString
Client::
name() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(ACRONYM_FIELD)) qFatal("Field is null");
   return _name;
}

bool
Client::
default_rate(int64_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(DEFAULT_RATE_FIELD)) return true;
   *rtn= _default_rate;
   return false;
}

int64_t
Client::
default_rate() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(DEFAULT_RATE_FIELD)) qFatal("Field is null");
   return _default_rate;
}

bool
Client::
default_charge_quantum(int32_t *rtn) const
/*****************************************************************
 * Access to field.
 */
{
   if(is_field_null(DEFAULT_CHARGE_QUANTUM_FIELD)) return true;
   *rtn= _default_charge_quantum;
   return false;
}

int64_t
Client::
default_charge_quantum() const
/*****************************************************************
 * Asserted access to field.
 */
{
   if(is_field_null(DEFAULT_CHARGE_QUANTUM_FIELD)) qFatal("Field is null");
   return _default_charge_quantum;
}

int
Client::
insert() 
/*****************************************************************
 * Convenience function for this app
 */
{
   return DbRec::insert(G.clientTable);
}

int
Client::
update()
/*****************************************************************
 * Convenience function for this app
 */
{
   return DbRec::update(G.clientTable);
}

int
Client::
remove()
/*****************************************************************
 * Remove Client record from database, after removing all projects.
 */
{
   QVector<int64_t> prj_id_vec;

   /* Get all of this Client's projects */
   int rtn = G.projectTable.fetchAll(prj_id_vec, QString("WHERE client_id=%1").arg(id()));
   if (rtn < 0) qFatal("Project_Table::fetchAll() failed!");

   /* Delete them */
   for (int i = 0; i < prj_id_vec.size(); ++i) {
      Project *prj= G.projectTable[prj_id_vec[i]];
      if(!prj) continue;
      prj->remove();
   }

   return DbRec::remove(G.clientTable);
}

static int
prj_cmp(const void *pp1, const void *pp2)
/* Project status comparator */
{
   const Project *prj1= *(const Project**)pp1,
                 *prj2= *(const Project**)pp2;

   if(prj2->currentState() < prj1->currentState()) return -1;
   if(prj2->currentState() == prj1->currentState()) return 0;
   return 1;
}

QIcon
Client::
currentStateIcon()
/*****************************************************************
 * Returns icon appropriate for tab.
 */
{
   QVector<int64_t> prj_id_vec;
   int rtn = G.projectTable.fetchAll(prj_id_vec, QString("WHERE client_id=%1").arg(id()));
   if (rtn < 0) qFatal("Project::fetchAll() failed!");

   /* If there are no projects, return empty icon */
   if(!prj_id_vec.count()) return QIcon();

   /* Get pointers to all the projects */
   Project *prj_vec[prj_id_vec.count()];
   for(int i= 0; i < prj_id_vec.count(); ++i) {
      prj_vec[i]= G.projectTable[prj_id_vec[i]];
      Q_ASSERT(prj_vec[i]);
   }

   /* sort the vector */
   ::qsort(prj_vec, prj_id_vec.count(), sizeof(Project*), prj_cmp);

   /* Return the first item */
   return prj_vec[0]->currentStateIcon();
}

