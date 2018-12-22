#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QIcon>
#include <QString>
#include <QHash>
#include "DbRec.h"

class Client;

class Client_Table : public QObject, public DbTable
/*********************************************
 * class for table that stores client records.
 */
{

   Q_OBJECT

public:

  Client_Table(QSqlDatabase &db);
  ~Client_Table();

  /* Eliminate default copy constructor and assignment operator */
  Client_Table(const Client_Table&)= delete;
  Client_Table& operator=(const Client_Table&)= delete;

  Client* operator[](int64_t id);

  /* Fetch a group of records */
   int fetchAll(QVector<int64_t> &idVec, const QString & sql_tail = QString());

   /* Create new instance of Client, return it cast to DbRec */
   virtual DbRec* factory() const;

signals:
   void sig_insert(int64_t client_id);
   void sig_update(int64_t client_id);
   void sig_remove(int64_t client_id);

private:
   virtual void emit_cast_sig_insert(DbRec*);
   virtual void emit_cast_sig_update(DbRec*);
   virtual void emit_cast_sig_remove(DbRec*);

   QHash<int64_t,Client*> _hash;

};



class Client : public DbRec
/*********************************************
 * Class for record of client_tbl
 */
{

public:

   Client();
   ~Client();

   /* Eliminate default copy constructor and assignment operator */
   Client(const Client&)= delete;
   Client& operator=(const Client&)= delete;

   void set_id(int64_t id);
   void set_acronym(const QString & acronym);
   void set_name(const QString & name);
   void set_default_rate(int64_t default_rate);
   void set_default_charge_quantum(int32_t default_charge_quantum);

   /* Returns true if field is null */
   bool id(int64_t *rtn) const;
   bool acronym(QString *rtn) const;
   bool name(QString *rtn) const;
   bool default_rate(int64_t *rtn) const;
   bool default_charge_quantum(int32_t *rtn) const;

   /* Asserted access */
   int64_t id() const;
   QString acronym() const;
   QString name() const;
   int64_t default_rate() const;
   int64_t default_charge_quantum() const;

   /* Overloaded DbRec functions */
   int insert();
   int update();
   int remove();

   /* Used to set tab icon */
   QIcon currentStateIcon();

private:

   friend class Client_Table;
#if 0
      // Smoke out any heap usage
      void * operator new   (size_t);
      void * operator new[] (size_t);
      void   operator delete   (void *);
      void   operator delete[] (void*);
#endif

   enum {
      /* Make sure the field enums start at 0, and increase sequentially */
      ID_FIELD = 0,
      ACRONYM_FIELD,
      NAME_FIELD,
      DEFAULT_RATE_FIELD,
      DEFAULT_CHARGE_QUANTUM_FIELD,
      UNKNOWN_FIELD
   };

   int64_t _id, _default_rate;
   int32_t _default_charge_quantum;
   QString _acronym, _name;

   const static DbField S_DbField_arr[];

};

#endif				// CLIENT_H
