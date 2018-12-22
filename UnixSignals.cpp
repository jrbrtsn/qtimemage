
#include <QDebug>
#include <QSocketNotifier>

#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "UnixSignals.h"

/* Static data has all bits cleared when execution of program begins */
int UnixSignals::S_sigsHandled,
    UnixSignals::S_nInstances, UnixSignals::S_fdArr[2];

/* This gets modified in Unix signal handler */
volatile int UnixSignals::S_sigsCaught;

/* This array maps the Unix signal number to the corresponding Qt signal we shall emit */
const UnixSignals::sigTuple UnixSignals::S_sigArr[] = {
   {
      .signo = SIGHUP,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_hup)
   }, {
      .signo = SIGINT,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_int)
   }, {
      .signo = SIGQUIT,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_quit)
   }, {
      .signo = SIGABRT,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_abrt)
   }, {
      .signo = SIGUSR1,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_usr1)
   }, {
      .signo = SIGUSR2,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_usr2)
   }, {
      .signo = SIGPIPE,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_pipe)
   }, {
      .signo = SIGALRM,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_alrm)
   }, {
      .signo = SIGTERM,
      .method = QMetaMethod::fromSignal(&UnixSignals::sig_term)
   }, {
      .signo = EOF
   }
};

void
UnixSignals::
handler(int signo)
/******************************************************************
 * Function called when Unix signal is raised.
 */
{
   /* Ignore if we haven't already handled this raised signal */
   if (S_sigsCaught & (1 << signo))
      return;

   /* Make a note that this signal was raised */
   S_sigsCaught |= (1 << signo);

   /* Write to the first socket to trigger socket notifier */
   char a = 1;
   ::write(S_fdArr[0], &a, sizeof(a));
}

UnixSignals::
UnixSignals(QObject * parent)
/******************************************************************
 * Constructor
 */
 : QObject(parent)
{
   /* For now, make sure there is only one instance of this class */
   Q_ASSERT(!S_nInstances);
   ++S_nInstances;

   /* Get an unnamed pipe */
   if (::socketpair(AF_UNIX, SOCK_STREAM, 0, S_fdArr))
      qFatal("Couldn't create socketpair!");

   /* Set both sockets for non-blocking I/O, close on exec() */
   int fcntlFlags;
   for (int ndx = 0; ndx < 2; ++ndx) {

      if ((fcntlFlags =::fcntl(S_fdArr[ndx], F_GETFD)) == EOF
	  ||::fcntl(S_fdArr[ndx], F_SETFL,
		    fcntlFlags | O_NONBLOCK | FD_CLOEXEC) == -1) {

	 qFatal("ERROR: fcntl()");
      }
   }

   /* Monitor the second socket for read ready */
   _sn = new QSocketNotifier(S_fdArr[1], QSocketNotifier::Read, this);
   connect(_sn, &QSocketNotifier::activated, this, &UnixSignals::sigsRaised);
}

UnixSignals::
~UnixSignals()
/******************************************************************
 * Destructor
 */
{
   Q_ASSERT(S_nInstances);
   --S_nInstances;

   /* Close the unamed pipe fd's */
   ::close(S_fdArr[0]);
   ::close(S_fdArr[1]);

   /* Restore default signal handling */
   const sigTuple *tpl;
   for (tpl = S_sigArr; tpl->signo != EOF; ++tpl) {
      struct sigaction sa;

      if (!(S_sigsHandled & (1 << tpl->signo)))
	 continue;

      sa.sa_handler = SIG_DFL;
      ::sigemptyset(&sa.sa_mask);
      if (::sigaction(tpl->signo, &sa, 0))
	 qFatal("sigaction() failed!");
   }
}

void
UnixSignals::
connectNotify(const QMetaMethod & signal)
/******************************************************************
 * Called when a Qt slot is connected to one of our Qt signals.
 */
{
   int signo = EOF;
   const sigTuple *tpl;

   /* Get the signo, if possible */
   for (tpl = S_sigArr; tpl->signo != EOF; ++tpl) {
      if (signal != tpl->method)
	 continue;

      signo = tpl->signo;
      break;
   }

   Q_ASSERT(signo != EOF);

   /* If we already handle that signal, nothing to be done */
   if (S_sigsHandled & (1 << signo))
      return;

   /* Note that we now handle this signal */
   S_sigsHandled |= (1 << signo);

   /* Install actual handler */
   struct sigaction sa;

   sa.sa_handler = UnixSignals::handler;
   ::sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sa.sa_flags |= SA_RESTART;
   if (::sigaction(signo, &sa, 0))
      qFatal("sigaction() failed!");

}

void				// slot
UnixSignals::
sigsRaised(int fd)
/******************************************************************
 * This gets called when a signal was raised.
 */
{
   int rtn;
   char buf[16];
   const sigTuple *tpl;

   /* Disable socket notifier */
   _sn->setEnabled(false);

   /* Empty any queued data */
   while ((rtn =::read(fd, buf, sizeof(buf))) >= 0) ;

   /* Check to make sure nothing unexpected happened */
   if (errno != EAGAIN && errno != EWOULDBLOCK)
      qFatal("read() error!");

   /* Emit any Qt signals which correspond with raised Unix signals */
   for (tpl = S_sigArr; tpl->signo != EOF; ++tpl) {

      /* emit corresponding Qt signals */
      while (S_sigsCaught & (1 << tpl->signo)) {
	 S_sigsCaught &= ~(1 << tpl->signo);
	 tpl->method.invoke(this, Qt::DirectConnection);
      }
   }

   /* Reenable socket notifier */
   _sn->setEnabled(true);
}
