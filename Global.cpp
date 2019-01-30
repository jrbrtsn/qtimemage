#include <map>
#include <QtGui>
#include <QMessageBox>
#include <QtWidgets/QMessageBox>

#include "qtimemage.h"
#include "MainWindow.h"
#include "configItem.h"
#include "util.h"

/* Object in which to store all global data */
Global G;

static struct {
  std::map<std::string,configItem*> *pCfg_map;
} S;

static void
init_static()
/************************************************
 * make sure static data is initialized.
 */
{
  if(S.pCfg_map) return;
  S.pCfg_map= new std::map<std::string,configItem*>();
}

/***************************************************************************/ 
/********************* Global **********************************************/ 
/***************************************************************************/ 

void
Global::
register_configItem(const char *key, configItem *ci)
/***************************************************************************
 * Register a configuration item to be parsed when the configuration is
 * loaded, and written when the configuration is stored.
 */ 
{
  init_static();
  S.pCfg_map->insert(std::pair<std::string,configItem*>(key, ci));
}

Global::
Global()
/***************************************************************************
 * Initialize our global data structure.
 */ 
: programTitle(tr("qTimeMage Time Card System"))
, programVersion("0.6.0")
, clientTable(db)
, projectTable(db)
, eventTable(db)
{
  
#ifdef _WIN32
   // TODO: what would this be?
  //dataDir= 
#else
  dataDir= QDir::homePath() + "/.local/share/qtimemage";
#endif
  if(!dataDir.exists()) {
    if(!dataDir.mkpath(dataDir.path())) {
      QString msg= tr("Cannot create data directory \"") + dataDir.path() + "\"";
      if(G.pApp) {
         QMessageBox::critical(0, G.programTitle, msg);
      } else {
         qDebug() << QString("Critical: %1").arg(msg);
      }
    }
  }
}

Global::
~Global()
/***************************************************************************
 * Free resources on exit.
 */ 
{
   // No need, because the program is exiting anyway
}

void
Global::
setVisibleTopLevelWindows(bool tf)
/***************************************************************************
 * Broadcast show/hide command for top level windows.
 */ 
{
   emit sig_setVisibleTopLevelWindows(tf);
}

void
Global::
userEventsChangeNotice()
/***************************************************************************
 * Broadcast show/hide command for top level windows.
 */ 
{
   emit sig_userEventsChangeNotice();
}

void
Global::
loadSettings()
/***************************************************************************
 * Load settings from the config file.
 */ 
{

  QString path= dataDir.filePath("qtimemage.conf");

  QFile f(path);
  if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) return;

  QTextStream stream(&f);

  while ( !stream.atEnd() ) {
    QString line, keywd;
    configItem *ci;
    int ndx;

    line = stream.readLine(); // line of text excluding '\n'
    if(!(ndx= line.indexOf(':'))) break;
    keywd= line.left(ndx);

    /* Look for keyword in hash table */
    auto it= S.pCfg_map->find(keywd.toStdString());
    if(it == S.pCfg_map->end()) {
      J_DBG << "Unrecognized config file entry: " << line;
//      Q_ASSERT(0);
      continue;
    }

    /* Assign config item object to pointer */
    ci= it->second;

    /* Parse the data after the keyword */
    if(ci->parse_line(line.mid(ndx+2))) {
      J_DBG << "Error parsing config file entry: " << line;
      Q_ASSERT(0);
      continue;
    }
  }
  
  f.close();
}

struct collect_ctxt {
  unsigned count;
  configItem **ci_arr;
};

void
Global::
saveSettings()
/***************************************************************************
 * Save settings to the config file.
 */ 
{
  emit sig_aboutToSaveSettings();

  QString path= dataDir.filePath("qtimemage.conf");

  QFile f(path);
  if(!f.open(QIODevice::WriteOnly|QIODevice::Text)) {
    QString msg= tr("Cannot open setting file \"") +path + "\"";
    QMessageBox::critical(pMw, G.programTitle, msg);
    return;
  }
  QTextStream stream(&f);

  for(auto it= S.pCfg_map->begin(); it != S.pCfg_map->end(); ++it) {
     std::string key= it->first;
     configItem *ci= it->second;
      if(!ci->has_info()) continue;
      stream << QString("%1: %2\n").arg(ci->key(), ci->write_line());
  }

  f.close();
#ifdef DEBUG
  J_DBG << "Settings saved at " << QDateTime::currentDateTime().toString();
#endif
}

void // static
Global::
msgHandler(QtMsgType type, const char *msg)
/***************************************************************************
 * Message handler for qDebug() and friends.
 */ 
{
static QFile lf;
static QTextStream strm;

  if(!lf.isOpen()) {
    lf.setFileName(G.dataDir.filePath("qtimemage.log"));
    if(!lf.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append)) {
      Q_ASSERT(0);
      /* we can't even open a log file? */
      return;
    }
    strm.setDevice(&lf);
  }

  strm << QDateTime::currentDateTime().toString() << " ";

  switch (type) {
    case QtInfoMsg: strm << "Information: "; break;
    case QtDebugMsg: strm << "Debug: "; break;
    case QtWarningMsg: strm << "Warning: "; break;
    case QtCriticalMsg: strm << "Critical: "; break;
    case QtFatalMsg: strm << "Fatal: "; break;
  }
  strm << msg << endl;
  if(type == QtFatalMsg) abort();
}

