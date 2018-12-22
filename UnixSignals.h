#ifndef UNIX_SIGNAlS_H
#define UNIX_SIGNAlS_H

#include <QObject>
#include <QSocketNotifier>
#include <QMetaMethod>

/* Convenience class to connect up Unix signals to Qt signal handlers */
class UnixSignals:public QObject {

 Q_OBJECT

public:
   UnixSignals(QObject * parent = 0);
   ~UnixSignals();

   /* Actual Unix signal handler function */
   static void handler(int sig);

    signals:
       /* connect() to these to get called when Unix signals are raised. */
   void sig_hup();
   void sig_int();
   void sig_quit();
   void sig_abrt();
   void sig_usr1();
   void sig_usr2();
   void sig_pipe();
   void sig_alrm();
   void sig_term();

private:

    QSocketNotifier * _sn;

   /* Gets called when there is action on the QSocketNotifier */
   void sigsRaised(int fd);

   /* Reimplemented from QObject so we know when to handle a signal */
   void connectNotify(const QMetaMethod & signal);

   /* Keep track of process-wide stuff.
    */
   static int S_sigsHandled, S_nInstances, S_fdArr[2];

   /* Keep track of which Unix signals have been raised */
   volatile static int S_sigsCaught;

   /* Used to map Unix signals to Qt signals listed above. */
   const static struct sigTuple {
      int signo;
      const QMetaMethod method;
   } S_sigArr[];
};

#endif				// UNIX_SIGNAlS_H
