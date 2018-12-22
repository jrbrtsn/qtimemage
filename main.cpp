#include <QApplication>
#include <QSqlDriver>
#include <QSharedMemory>
#include <QDir>
#include <QTimer>

#include "qtimemage.h"
#include "MainWindow.h"
#ifndef _WIN32
#include "UnixSignals.h"
#endif

static void
quit_h()
/*************************************************************
 * This called when a Unix signal is caught indicating time
 * to exit.
 */
{
   G.pApp->quit();
}

int
main(int argc, char *argv[])
/*************************************************************
 * Program execution begins here.
 */
{
   int rtn;
   G.pApp= new QApplication(argc, argv);

   /* Use a shared memory segment to enforce singleton application behavior */
   QSharedMemory sm;

   {				/* Make a system wide unique key for this user and application */
      QByteArray user = qgetenv("USER");
      sm.setKey(QString(user).append(".qtimemage"));
   }

   /* Check to see if another process is already running */
   if (sm.attach()) {
      qWarning("Another instance of qtimemage appears to be running already!");
      return 1;
   }

   /* Use a keyed shared memory segment to protect from the user running multiple instances of this program. */
   if (!sm.create(1))
      qFatal("QSystemSemaphore::create() failed!");

#ifndef _WIN32
   /* Set up Unix signal handling */
   UnixSignals Ush(G.pApp);

   /* Arrange for our quit handler to be called for various signals */
   QObject::connect(&Ush, &UnixSignals::sig_hup, &quit_h);
   QObject::connect(&Ush, &UnixSignals::sig_int, &quit_h);
   QObject::connect(&Ush, &UnixSignals::sig_quit, &quit_h);
   QObject::connect(&Ush, &UnixSignals::sig_abrt, &quit_h);
   QObject::connect(&Ush, &UnixSignals::sig_pipe, &quit_h);
   QObject::connect(&Ush, &UnixSignals::sig_term, &quit_h);
#endif

   /* Get the sqlite3 database driver */
   G.db = QSqlDatabase::addDatabase("QSQLITE");
   if (!G.db.isValid()) Q_ASSERT(0);

   // TODO: if no data exists, create the directory with an empty database

   /* Open the database */
#ifdef _WIN32
   // TODO: where would this be for Windows?
#else
   G.db.setDatabaseName(QDir::home().filePath(".local/share/qtimemage/db"));
#endif
   if (!G.db.open()) Q_ASSERT(0);

   /* Make sure the driver will give us the last insert primary key */
   if (!G.db.driver()->hasFeature(QSqlDriver::LastInsertId)) Q_ASSERT(0);

   G.loadSettings();
   G.pMw= new MainWindow;
   G.pMw->show();

   /* Save the settings at least every 5 minutes, in case of crashes, power failures, etc. */
   QTimer settingsTimer;
   G.connect(&settingsTimer, SIGNAL(timeout()), SLOT(saveSettings()));
   settingsTimer.setInterval(5*60*1000);
   settingsTimer.start();

   rtn = G.pApp->exec();

   /* Save the most recent settings */
   G.saveSettings();

   /* Cleanup */
   if (G.db.isOpen()) {
      G.db.close();
   }

   return rtn;
}
