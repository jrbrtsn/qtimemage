#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>
#include <QIcon>
#include <QDateTime>
#include <QHash>

#include "DbRec.h"
#include "Event.h"

#define PROJECT_REPORT_MARGIN_SECS 172800

class Project;

class Project_Table : public QObject, public DbTable
/*********************************************
 * class for table that stores project records.
 */
{

   Q_OBJECT

public:

  Project_Table(QSqlDatabase &db);
  ~Project_Table();

  /* Eliminate default copy constructor and assignment operator */
  Project_Table(const Project_Table&)= delete;
  Project_Table& operator=(const Project_Table&)= delete;

  Project* operator[](int64_t id);

   int fetchAll(QVector<int64_t> &idVec, const QString & sql_tail = QString());

   /* Create new instance of Project, return it cast to DbRec */
   virtual DbRec* factory() const;

signals:
   void sig_insert(int64_t);
   void sig_update(int64_t);
   void sig_remove(int64_t);

private:
   virtual void emit_cast_sig_insert(DbRec*);
   virtual void emit_cast_sig_update(DbRec*);
   virtual void emit_cast_sig_remove(DbRec*);

   QHash<int64_t,Project*> _hash;
};


class Project : public DbRec {

public:
   Project();
   ~Project();

   /* Eliminate default copy constructor and assignment operator */
   Project(const Project&)= delete;
   Project& operator=(const Project&)= delete;

   enum State {
      ERROR,
      STOPPED,
      PAUSED,
      RECORDING
   };

   /* Note: These Event related functions all use G.eventTable */
   enum State currentState() const;
   QIcon currentStateIcon() const;
   void Start();
   void Stop();
   void Pause();

   /* Set field values */
   void set_id(int64_t id);
   void set_client_id(int64_t id);
   void set_project_id(int64_t id);
   void set_title(const QString & title);
   void set_rate(int64_t rate);
   void set_charge_quantum(int32_t charge_quantum);

   /* Returns true if field is null */
   bool id(int64_t *rtn) const;
   bool client_id(int64_t *rtn) const;
   bool project_id(int64_t *rtn) const;
   bool title(QString *rtn) const;
   bool rate(int64_t *rtn) const;
   bool charge_quantum(int32_t *rtn) const;

   /* Asserted access functions */
   int64_t id() const;
   int64_t client_id() const;
   QString title() const;
   int64_t rate() const;
   int32_t charge_quantum() const;
   Project* parentProject() const;

   /* Note: these function use assumed tables from G */
   QString longTitle() const;
   QString FQLongTitle() const;
   int reportTime(int64_t begin_secs, int64_t end_secs);
   int reportTime(const QDateTime &begin, const QDateTime &end)
   { return reportTime(begin.toSecsSinceEpoch(), end.toSecsSinceEpoch());}

   /* Overloaded DbRec functions */
   int insert();
   int update();
   int remove();

private:

   friend class Project_Table;
#if 0
      // Limit heap functions
      void * operator new   (size_t);
      void * operator new[] (size_t);
      void   operator delete   (void *);
      void   operator delete[] (void*);
#endif

   /* Note: These Event related functions all use G.eventTable */
   void insertEvent(int eventTypeEnum);
   Event* lastEvent() const;

   enum {
      /* Make sure the field enums start at 0, and increase sequentially */
      ID_FIELD = 0,
      CLIENT_ID_FIELD,
      PROJECT_ID_FIELD,
      TITLE_FIELD,
      RATE_FIELD,
      CHARGE_QUANTUM_FIELD,
      UNKNOWN_FIELD
   };

   int64_t _id,
           _client_id,
           _project_id;

   QString _title;

   int64_t _rate;
   int32_t _charge_quantum;

   const static DbField S_DbField_arr[];

};

#endif // PROJECT_H
