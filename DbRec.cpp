#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include "qtimemage.h"
#include "DbRec.h"
#include "util.h"

const char*
DbField::
type2str(int type)
/*******************************************************************************
 * Give a readable version of the DbField type.
 */
{
   switch (type) {

   case PKEY_TYPE:
      return "PKEY";
   case BOOL_TYPE:
      return "BOOL";
   case INT32_TYPE:
      return "INT32";
   case UINT32_TYPE:
      return "UINT32";
   case INT64_TYPE:
      return "INT64";
   case FLOAT_TYPE:
      return "FLOAT";
   case DOUBLE_TYPE:
      return "DOUBLE";
   case STRING_TYPE:
      return "STRING";
   case ENUM_TYPE:
      return "ENUM";
   case BYTEARR_TYPE:
      return "BYTEARR";
   case DATETIME_TYPE:
      return "DATETIME";
   case DATE_TYPE:
      return "DATE";
   case TIME_TYPE:
      return "TIME";
   case INET_TYPE:
      return "INET";
   }

   return "Unknown type!";
}

/*******************************************************************************/
/************************ DbTable **********************************************/
/*******************************************************************************/

DbTable::
DbTable(QSqlDatabase &db, const char *tblName, const DbField dbf_arr[])
/*******************************************************************************
 * Constructor
 */
: _db(db)
,_name(tblName)
, _DbField_arr(dbf_arr)
{

   /* See if a primary key can be found */
   const DbField *f;
   for (f = dbf_arr; f->colName; ++f) {
      if (f->type == DbField::PKEY_TYPE) {
	 _pKey = f;
	 break;
      }
   }
}

int
DbTable::
fetchAll(DbRec * rtnVec[], unsigned vecSz, const QString & sql_tail) const
/*******************************************************************************
 * Fetch all records from a table, put results into a vector of DbRec pointers.
 * returns:
 *      EOF  if vecSz is too small
 *      EOF-1  other errors
 *      Number of records retrieved, including 0.
 */
{
   int nLoaded = 0, rtn = EOF - 1;

   /* Form the query string */
   QString qstr("SELECT ");
   for (const DbField * f = _DbField_arr; f->colName; ++f) {
      if (f != _DbField_arr)
	 qstr.append(", ");
      qstr.append(f->colName);
   }

   qstr.append(" FROM ");
   qstr.append(_name);

   if (!sql_tail.isNull()) {
      qstr.append(" ");
      qstr.append(sql_tail);
   }

//   J_DBG_FN << qstr;

   /* Prepare query */
   QSqlQuery q(_db);
   if (!q.prepare(qstr))
      Q_ASSERT(0);

   /* Perform query */
   if (!q.exec()) {
      qDebug() << "Query \"" << qstr << "\" returned error: " << q.lastError().
	  text();
      goto abort;
   }

   Q_ASSERT(q.isSelect());

   /* SQLite driver doesn't know how many rows are in result, so we count
    * as we load new objects with data.
    */
   for (nLoaded = 0; q.next(); ++nLoaded) {
      DbRec *obj;
      if (nLoaded == (int)vecSz) {
	 rtn = EOF;
	 goto abort;
      }
      Q_ASSERT(q.isValid());
      obj = factory();
      rtnVec[nLoaded] = obj;
      if (obj->fetch(q, _DbField_arr)) Q_ASSERT(0);
   }

//   J_DBG_FN << "nLoaded=" << nLoaded;
   rtn = nLoaded;

 abort:
   /* Clean up on the way out */
   if (rtn < 0) {
      for (int i = 0; i < nLoaded; ++i) {
	 delete rtnVec[i];
      }
   }
   return rtn;
}

int
DbTable::
fetchAll_r(
      DbRec * rtnVec[],
      unsigned vecSz,
      const QString &sql_pfx,
      const char *tmp_tbl,
      const char *tmp_pKey
      ) const
/*******************************************************************************
 * Fetch all records from a table, using WITH prefix
 * into a vector of DbRec pointers.
 * returns:
 *      EOF  if vecSz is too small
 *      EOF-1  other errors
 *      Number of records retrieved, including 0.
 */
{
   int nLoaded = 0, rtn = EOF - 1;

   /* Form the query string */
   QString qstr= sql_pfx + " SELECT DISTINCT ";
   for (const DbField * f = _DbField_arr; f->colName; ++f) {

      if (f != _DbField_arr)
	 qstr.append(", ");

      qstr.append(_name);
      qstr.append('.');
      qstr.append(f->colName);
      qstr.append(" AS ");
      qstr.append(f->colName);
   }

   qstr.append(
     QString(" FROM %1, %2 WHERE %2.%3 = %1.%4")
      .arg(tmp_tbl)
      .arg(_name)
      .arg(_pKey->colName)
      .arg(tmp_pKey)
         );

J_DBG_FN << qstr;

   /* Prepare query */
   QSqlQuery q(_db);
   if (!q.prepare(qstr))
      Q_ASSERT(0);

   /* Perform query */
   if (!q.exec()) {
      qDebug() << "Query \"" << qstr << "\" returned error: " << q.lastError().
	  text();
      goto abort;
   }

   Q_ASSERT(q.isSelect());

   /* SQLite driver doesn't know how many rows are in result, so we count
    * as we load new objects with data.
    */
   for (nLoaded = 0; q.next(); ++nLoaded) {
      DbRec *obj;
      if (nLoaded == (int)vecSz) {
	 rtn = EOF;
	 goto abort;
      }
      Q_ASSERT(q.isValid());
      obj = factory();
      rtnVec[nLoaded] = obj;
      if (obj->fetch(q, _DbField_arr)) Q_ASSERT(0);
   }

//   J_DBG_FN << "nLoaded=" << nLoaded;
   rtn = nLoaded;

 abort:

   /* Clean up on the way out */
   if (rtn < 0) {
      for (int i = 0; i < nLoaded; ++i) {
	 delete rtnVec[i];
      }
   }
   return rtn;
}

/*******************************************************************************/
/************************ DbRec ************************************************/
/*******************************************************************************/

DbRec::
DbRec(void *offset_this, const DbField dbf_arr[])
/*******************************************************************************
 * Constructor
 */
: _offset_this((char *)(offset_this))
, _DbField_arr(dbf_arr)
, _nullFlags(-1LL)
{
//   J_DBG_FN;
   Q_ASSERT(offset_this);
}

DbRec&
DbRec::
operator=(const DbRec &src)
/*******************************************************************************
 * Assignment operator only copies nullFlags
 */
{
   _nullFlags= src._nullFlags;
   return *this;
}


DbRec::
~DbRec()
/*******************************************************************************
 * Destructor
 */
{
}

int
DbRec::
stow(QSqlQuery & q, const DbField * f)
/*******************************************************************************
 * Store our value from the current query row.
 */
{
   Q_ASSERT(q.isValid());

   QVariant qv = q.value(f->colEnum);

   Q_ASSERT(qv.isValid());

   /* If the field is null, set is thusly */
   if (qv.isNull()) {
      set_field_null(true, f->colEnum);
      return 0;
   }

   /* Otherwise, field is not null ... */
   set_field_null(false, f->colEnum);

   /* Need to store the values with some type safety */

   bool success(true);

   switch (f->type) {
      case DbField::PKEY_TYPE:
      case DbField::INT64_TYPE:{
            int64_t *pVal = offset_int64(f->offset);
            *pVal = qv.toLongLong(&success);
            Q_ASSERT(success);
         }
         break;

      case DbField::INT32_TYPE:{
            int32_t *pVal = offset_int32(f->offset);
            *pVal = qv.toLongLong(&success);
            Q_ASSERT(success);
         }
         break;

      case DbField::UINT32_TYPE:{
            uint32_t *pVal = offset_uint32(f->offset);
            *pVal = qv.toUInt(&success);
            Q_ASSERT(success);
         }
         break;

      case DbField::FLOAT_TYPE:{
            float *pVal = offset_float(f->offset);
            *pVal = qv.toFloat(&success);
            Q_ASSERT(success);
         } break;

      case DbField::DOUBLE_TYPE:{
            double *pVal = offset_double(f->offset);
            *pVal = qv.toDouble(&success);
            Q_ASSERT(success);
         } break;

      case DbField::STRING_TYPE:
      case DbField::ENUM_TYPE:{
            QString *pVal = offset_QString(f->offset);
            *pVal = qv.toString();
         }
         break;

      case DbField::BYTEARR_TYPE:{
            QByteArray *pVal = offset_QByteArray(f->offset);
            *pVal = qv.toByteArray();
         }
         break;

      case DbField::DATETIME_TYPE:{
            QDateTime *pVal = offset_QDateTime(f->offset);
            *pVal = qv.toDateTime();
         }
         break;

      case DbField::DATE_TYPE:{
            QDate *pVal = offset_QDate(f->offset);
            *pVal = qv.toDate();
         }
         break;

      case DbField::TIME_TYPE:{
            QTime *pVal = offset_QTime(f->offset);
            *pVal = qv.toTime();
         }
         break;

      default:
         Q_ASSERT(0);
   }

   Q_ASSERT(success);
   return 0;
}

int
DbRec::
bind(QSqlQuery & q, const DbField * f)
/*******************************************************************************
 * Bind the field value to the query.
 */
{

   /* If the field is null, bind the null QString */
   if (is_field_null(f->colEnum)) {
      q.addBindValue(QString());
      return 0;
   }

   switch (f->type) {
   case DbField::PKEY_TYPE:
   case DbField::INT64_TYPE:{
	 int64_t *pVal = offset_int64(f->offset);
	 q.addBindValue((long long int)(*pVal));
      } break;

   case DbField::INT32_TYPE:{
	 int32_t *pVal = offset_int32(f->offset);
	 q.addBindValue(*pVal);
      }
      break;

   case DbField::UINT32_TYPE:{
	 uint32_t *pVal = offset_uint32(f->offset);
	 q.addBindValue(*pVal);
      }
      break;

   case DbField::FLOAT_TYPE:{
	 float *pVal = offset_float(f->offset);
	 q.addBindValue(*pVal);
      } break;

   case DbField::DOUBLE_TYPE:{
	 double *pVal = offset_double(f->offset);
	 q.addBindValue(*pVal);
      } break;

   case DbField::STRING_TYPE:
   case DbField::ENUM_TYPE:{
	 QString *pVal = offset_QString(f->offset);
	 q.addBindValue(*pVal);
      }
      break;

   case DbField::BYTEARR_TYPE:{
	 QByteArray *pVal = offset_QByteArray(f->offset);
	 q.addBindValue(*pVal);
      }
      break;

   case DbField::DATETIME_TYPE:{
	 QDateTime *pVal = offset_QDateTime(f->offset);
	 q.addBindValue(*pVal);
      }
      break;

   case DbField::DATE_TYPE:{
	 QDate *pVal = offset_QDate(f->offset);
	 q.addBindValue(*pVal);
      }
      break;

   case DbField::TIME_TYPE:{
	 QTime *pVal = offset_QTime(f->offset);
	 q.addBindValue(*pVal);
      }
      break;

   default:
      Q_ASSERT(0);
   }
   return 0;
}

int
DbRec::
fetch(DbTable& tbl)
/*******************************************************************************
 * Fetch this record in the table, assuming the primary key has been set.
 */
{
   int rtn = 1;
   unsigned n;
   const DbField *f;
   if (!tbl.pKey() || is_field_null(tbl.pKey()->colEnum)) Q_ASSERT(0);

   /* Form the query string */
   QString qstr("SELECT ");

   /* Get all the columns names in the query */
   for (n = 0, f = tbl.DbField_arr(); f->colName; ++f) {
      /* Because fetch(QSqlQuery & q, const DbField dbf_arr[]) will get the primary key, do not skip it here!!! */
      if (n) qstr.append(", ");
      qstr.append(f->colName);
      ++n;
   }
   qstr.append(" FROM ");
   qstr.append(tbl.name());
   qstr.append(" WHERE ");
   qstr.append(tbl.pKey()->colName);
   qstr.append(" = ?");

//   J_DBG_FN << qstr;

   /* Bind values for the query */
   QSqlQuery q(tbl.db());
   if (!q.prepare(qstr)) Q_ASSERT(0);

   /* Now bind the primary key */
   if (bind(q, tbl.pKey())) Q_ASSERT(0);

   /* Perform the query */
   if (!q.exec()) {
      qDebug() << q.lastError().text();
      goto abort;
   }

   Q_ASSERT(q.isSelect());

   /* This record may not exist in the table */
   if(!q.next()) goto abort;

   Q_ASSERT(q.isValid());
   /* NOTE: this will try to get all fields, even if you did not ask for them!!! */
   if (fetch(q, _DbField_arr)) Q_ASSERT(0);

   rtn = 0;

 abort:

   return rtn;
}

int
DbRec::
insert(DbTable &tbl)
/*******************************************************************************
 * Insert this record into a table.
 */
{

   int rtn = 1;

   /* If the schema contains a primary key, make sure it is currently null */
   if (tbl.pKey()) {
      Q_ASSERT(is_field_null(tbl.pKey()->colEnum));
   }
   // TODO: store & hash resulting query for a given DbField_arr address and database connection, avoid rebuilding same query.

   /* Form the query string */
   QString qstr("INSERT INTO ");
   qstr.append(tbl.name());
   qstr.append(" (");
   const DbField *f;
   unsigned n;
   for (n = 0, f = tbl.DbField_arr(); f->colName; ++f) {
      /* We don't specify the primary key */
      if (f->type == DbField::PKEY_TYPE)
	 continue;
      if (n)
	 qstr.append(", ");
      qstr.append(f->colName);
      ++n;
   }

   qstr.append(") VALUES (");
   for (n = 0, f = tbl.DbField_arr(); f->colName; ++f) {
      /* We don't specify the primary key */
      if (f->type == DbField::PKEY_TYPE)
	 continue;
      if (n)
	 qstr.append(", ");
      qstr.append("?");
      ++n;
   }
   qstr.append(")");

//J_DBG_FN << qstr;

   /* Bind values for the query */
   QSqlQuery q(tbl.db());
   if (!q.prepare(qstr))
      Q_ASSERT(0);
   for (f = tbl.DbField_arr(); f->colName; ++f) {

      /* We don't bind the primary key */
      if (f->type == DbField::PKEY_TYPE)
	 continue;

      /* Bind value to the query */
      if (bind(q, f)) Q_ASSERT(0);
   }

   /* Perform the query */
   if (!q.exec()) {
      qDebug() << q.lastError().text();
      goto abort;
   }

   /* Get the primary key, if there is one */
   if (tbl.pKey()) {
      QVariant var = q.lastInsertId();
      Q_ASSERT(var.isValid());
      Q_ASSERT(!var.isNull());
      int64_t *pVal = offset_int64(tbl.pKey()->offset);
      bool success;
      *pVal = var.toLongLong(&success);
      Q_ASSERT(success);
      set_field_null(false, tbl.pKey()->colEnum);
   }

   tbl.emit_cast_sig_insert(this);
   rtn = 0;

 abort:
   return rtn;
}

int
DbRec::
remove(DbTable& tbl)
/*******************************************************************************
 * Remove this record from the table.
 */
{
   int rtn = 1;

   /* Form the query string */
   QString qstr("DELETE FROM ");
   qstr.append(tbl.name());
   qstr.append(" WHERE ");

   // TODO: See if record exists

   if (tbl.pKey()) {
      /* Delete by primary key */
      Q_ASSERT(!is_field_null(tbl.pKey()->colEnum));
      qstr.append(tbl.pKey()->colName);
      qstr.append(" = ?");

   } else {
      /* TODO: query to see if this is unique, then either return error or delete by
         all current values.
       */
      Q_ASSERT(0);
   }

//   J_DBG_FN << qstr;

   /* Bind values for the query */
   QSqlQuery q(tbl.db());
   if (!q.prepare(qstr))
      Q_ASSERT(0);

   if (tbl.pKey()) {
      Q_ASSERT(!is_field_null(tbl.pKey()->colEnum));
      /* Add the value of the primary key */
      int64_t *pVal = offset_int64(tbl.pKey()->offset);
      q.addBindValue((long long int)(*pVal));

   } else {
      /* TODO: query to see if this is unique, then either return error or delete by
         all current values.
       */
      Q_ASSERT(0);
   }

   /* Perform the query */
   if (!q.exec()) {
      qDebug() << q.lastError().text();
      goto abort;
   }

   tbl.emit_cast_sig_remove(this);

   /* If there is a primary key, make it null */
   if (tbl.pKey()) {
      set_field_null(true, tbl.pKey()->colEnum);
   }

   rtn = 0;

 abort:
   return rtn;
}

int
DbRec::
update(DbTable& tbl)
/*******************************************************************************
 * Update this record in the table.
 */
{
   int rtn = 1;
   unsigned n;
   const DbField *f;
   if (!tbl.pKey() || is_field_null(tbl.pKey()->colEnum))
      Q_ASSERT(0);

   /* Form the query string */
   QString qstr("UPDATE ");
   qstr.append(tbl.name());
   qstr.append(" SET ");

   /* Get all the columns names in the query */
   for (n = 0, f = tbl.DbField_arr(); f->colName; ++f) {
      /* We don't specify the primary key */
      if (f->type == DbField::PKEY_TYPE)
	 continue;
      if (n)
	 qstr.append(", ");
      qstr.append(f->colName);
      qstr.append(" = ?");
      ++n;
   }

   qstr.append(" WHERE ");
   qstr.append(tbl.pKey()->colName);
   qstr.append(" = ?");

//   J_DBG_FN << qstr;

   /* Bind values for the query */
   QSqlQuery q(tbl.db());
   if (!q.prepare(qstr)) Q_ASSERT(0);

   for (f = tbl.DbField_arr(); f->colName; ++f) {

      /* We don't bind the primary key */
      if (f->type == DbField::PKEY_TYPE)
	 continue;

      /* Bind value to the query */
      if (bind(q, f)) Q_ASSERT(0);
   }

   /* Now bind the primary key */
   if (bind(q, tbl.pKey())) Q_ASSERT(0);

   /* Perform the query */
   if (!q.exec()) {
      qDebug() << q.lastError().text();
      goto abort;
   }

   tbl.emit_cast_sig_update(this);

   rtn = 0;

 abort:

   return rtn;
}

int
DbRec::
fetch(QSqlQuery & q, const DbField dbf_arr[])
/*******************************************************************************
 * Fetch this record from the supplied query result
 */
{
   int rtn = 1;
   const DbField *f;
   for (f = dbf_arr; f->colName; ++f) {
      if (stow(q, f)) Q_ASSERT(0);
   }

   rtn = 0;

//abort:

   return rtn;
}

void
DbRec::
set_nullFlags(int64_t flags)
/*******************************************************************************
 * Set the null flags
 */
{
   _nullFlags = flags;
}

void
DbRec::
set_field_null(bool tf, int fieldEnum)
/*******************************************************************************
 * Set or clear the null status of a single field.
 */
{
   if (tf) {
      /* Set the bit */
      _nullFlags |= (1LL << fieldEnum);
   } else {
      /* Clear the bit */
      _nullFlags &= ~(1LL << fieldEnum);
   }
}

bool
DbRec::
is_field_null(int fieldEnum) const
/*******************************************************************************
 * Discover if a field is currently null.
 */
{
   Q_ASSERT(fieldEnum >= 0 && fieldEnum < 64);
   return _nullFlags & (1LL << fieldEnum);
}


QDebug
operator<<(QDebug dbg, const DbRec &o)
/*****************************************************************
 * Debug printer
 */   
{ 
   const DbField *f;
   dbg << "{\n";

   for (f = o._DbField_arr; f->colName; ++f) {
      dbg << *f << ", ";
      if (o.is_field_null(f->colEnum)) {
	 dbg << "NULL";
      } else {
         dbg << "Value= ";

	 switch (f->type) {

            case DbField::PKEY_TYPE:
            case DbField::INT64_TYPE:{
                  const int64_t *pVal = o.offset_int64(f->offset);
                  dbg << *pVal;
               }
               break;

            case DbField::INT32_TYPE:{
                  const int32_t *pVal = o.offset_int32(f->offset);
                  dbg << *pVal;
               }
               break;

            case DbField::STRING_TYPE:{
                  const QString *pVal = o.offset_QString(f->offset);
                  dbg << *pVal;
               }
               break;

               // TODO: all other types

            default:
               Q_ASSERT(0);

	  }
       }
      dbg << '\n';
   }
   dbg << "}" << endl;
   return dbg;
}

/*******************************************************************************/
/**************************** DbField ******************************************/
/*******************************************************************************/
QDebug
operator<<(QDebug dbg, const DbField &o)
/*****************************************************************
 * Debug printer
 */   
{ 
   dbg << "\tname= " << o.colName << ", type= " << DbField::type2str(o.type) << ", colEnum= " << o.colEnum << ", offset= " << o.offset;
       return dbg;
}
