#ifndef DBREC_H
#define DBREC_H

#include <stdint.h>
#include <QNetworkAddressEntry>
#include <QString>
#include <QDebug>
#include <QSqlDatabase>
#include <QObject>

struct DbField {

   enum Type {
      PKEY_TYPE,		// int64_t
      BOOL_TYPE,		// int
      INT32_TYPE,		// int32_t
      UINT32_TYPE,		// unit32_t
      INT64_TYPE,		// int64_t
      FLOAT_TYPE,		// float
      DOUBLE_TYPE,		// double
      STRING_TYPE,		// QString
      ENUM_TYPE,		// QString
      BYTEARR_TYPE,		// QByteArr
      DATETIME_TYPE,		// QDateTime
      DATE_TYPE,		// QDate
      TIME_TYPE,		// QTime
      INET_TYPE,		// QNetworkAddressEntry
      UNKOWN_TYPE
   };

   const char *colName;
   enum Type type;
   int colEnum;
   size_t offset;

   static const char *type2str(int type);
};

#define class_offsetof(cl, m) \
    (size_t)(&((cl*)0)->m)

/* Forward declaration */
class DbRec;

/* One of these for each table which will be used */
class DbTable {

public:

   DbTable(QSqlDatabase &db, const char *tblName, const DbField dbf_arr[]);

  /* Eliminate default copy constructor and assignment operator */
   DbTable(const DbTable&)= delete;
   DbTable& operator=(const DbTable&)= delete;

   int fetchAll(DbRec * rtnVec[], unsigned vecSz, const QString & sql_tail) const;

   inline const DbField *DbField_arr() const {
      return _DbField_arr;
   } inline const DbField *pKey() const {
      return _pKey;
   } inline const QString & name() const {
      return _name;
   }
   inline QSqlDatabase& db() {
      return _db;
   }

protected:

   virtual DbRec* factory() const =0;

   friend class DbRec;
   virtual void emit_cast_sig_insert(DbRec*)=0;
   virtual void emit_cast_sig_update(DbRec*)=0;
   virtual void emit_cast_sig_remove(DbRec*)=0;

private:

   QSqlDatabase &_db;
   QString _name;
   const DbField *_DbField_arr, *_pKey;
};

QDebug operator<<(QDebug dbg, const DbField &o);

/* One of these for each record with which we will be working */
class DbRec {


public:

   DbRec& operator=(const DbRec&);
   virtual ~DbRec();

  /* Eliminate default copy constructor */
   DbRec(const DbRec&)= delete;

   int insert(DbTable &tbl);
   int remove(DbTable &tbl);
   int update(DbTable &tbl);
   int fetch(DbTable &tbl);

   void set_field_null(bool tf, int fieldEnum);
   bool is_field_null(int fieldEnum) const;
   void set_nullFlags(int64_t flags);

protected:

    DbRec(void *offset_this, const DbField dbf_arr[]);

private:

   friend class DbTable;

   int fetch(QSqlQuery& q, const DbField dbf_arr[]);
   int bind(QSqlQuery & q, const DbField * f);
   int stow(QSqlQuery & q, const DbField * f);

   inline int *offset_bool(size_t offset) {
      return (int *)(_offset_this + offset);
   }
   inline const int *offset_bool(size_t offset) const {
      return (const int *)(_offset_this + offset);
   }
   inline int32_t *offset_int32(size_t offset) {
      return (int32_t *) (_offset_this + offset);
   }
   inline const int32_t *offset_int32(size_t offset) const {
      return (const int32_t *) (_offset_this + offset);
   }
   inline uint32_t *offset_uint32(size_t offset) {
      return (uint32_t *) (_offset_this + offset);
   }
   inline int64_t *offset_int64(size_t offset) {
      return (int64_t *) (_offset_this + offset);
   }
   inline const int64_t *offset_int64(size_t offset) const {
      return (const int64_t *) (_offset_this + offset);
   }
   inline float *offset_float(size_t offset) {
      return (float *)(_offset_this + offset);
   }
   inline double *offset_double(size_t offset) {
      return (double *)(_offset_this + offset);
   }
   inline QString *offset_QString(size_t offset) {
      return (QString *) (_offset_this + offset);
   }
   inline const QString *offset_QString(size_t offset) const {
      return (const QString *) (_offset_this + offset);
   }
   inline QByteArray *offset_QByteArray(size_t offset) {
      return (QByteArray *) (_offset_this + offset);
   }
   inline QDateTime *offset_QDateTime(size_t offset) {
      return (QDateTime *) (_offset_this + offset);
   }
   inline QDate *offset_QDate(size_t offset) {
      return (QDate *) (_offset_this + offset);
   }
   inline QTime *offset_QTime(size_t offset) {
      return (QTime *) (_offset_this + offset);
   }
   inline QNetworkAddressEntry *offset_QNetworkAddressEntry(size_t offset) {
      return (QNetworkAddressEntry *) (_offset_this + offset);
   }

   friend QDebug operator<<(QDebug dbg, const DbRec &o);

   char *_offset_this;
   const DbField *_DbField_arr;
   int64_t _nullFlags;

};

QDebug operator<<(QDebug dbg, const DbRec &o);

#endif				// DBREC_H
