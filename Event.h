#ifndef EVENT_H
#define EVENT_H

#include <QObject>
#include <QString>
#include <QMetaType>
#include <QHash>
#include "DbRec.h"

class QDateTime;
class Event;

class Event_Table : public QObject, public DbTable
/*********************************************
 * class for table that stores project records.
 */
{
   Q_OBJECT

public:

  Event_Table(QSqlDatabase &db);
  ~Event_Table();

  /* Eliminate default copy constructor and assignment operator */
  Event_Table(const Event_Table&)= delete;
  Event_Table& operator=(const Event_Table&)= delete;

  Event* operator[](int64_t id);

  /* Fetch a group of records */
   int fetchAll(QVector<int64_t> &idVec, const QString & sql_tail = QString());

   /* Create new instance of Event, return it cast to DbRec */
   virtual DbRec* factory() const;

signals:
   void sig_insert(int64_t event_id);
   void sig_update(int64_t event_id);
   void sig_remove(int64_t event_id);

private:
   virtual void emit_cast_sig_insert(DbRec*);
   virtual void emit_cast_sig_update(DbRec*);
   virtual void emit_cast_sig_remove(DbRec*);

   QHash<int64_t,Event*> _hash;

};


class Event : public DbRec {

public:

   enum Type {
      START=0,
      STOP,
      PAUSE
   };

   Event();
   ~Event();

   /* Eliminate default copy constructor */
   Event(const Event&)= delete;
   Event& operator=(const Event&)= delete;

   /* Set the data fields */
   void set_id(int64_t id);
   void set_project_id(int64_t prj_id);
   void set_when_ts(int64_t secs);
   void set_when_ts(const QDateTime &when);
   void set_type_enum(int32_t type);

   /* Returns true if field is null */
   bool id(int64_t *rtn) const;
   bool project_id(int64_t *rtn) const;
   bool when_ts(int64_t *rtn) const;
   bool when_ts(QDateTime *rtn) const;
   bool type_enum(int *rtn) const;

   /* Asserted access functions */
   int64_t id() const;
   int64_t project_id() const;
   int64_t when_ts() const;
   int type_enum() const;
   const char* type_str() const;

   /* Overloaded DbRec functions */
   int insert();
   int update();
   int remove();

private:

   enum {
      /* Make sure the field enums start at 0, and increase sequentially */
      ID_FIELD = 0,
      PROJECT_ID_FIELD,
      WHEN_TS_FIELD,
      TYPE_ENUM_FIELD,
      UNKNOWN_FIELD
   };

   int64_t _id,
           _project_id,
           _when_ts;

   int32_t _type_enum;

   friend class Event_Table;
   const static DbField S_DbField_arr[];

};

#endif // EVENT_H
