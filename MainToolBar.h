#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include <QToolBar>
#include <QSystemTrayIcon>

class QAction;
class Client;
class Project;
class ProjectComboBox;

class MainToolBar : public QToolBar {

   Q_OBJECT

public:
   MainToolBar(QWidget *parent);
   ~MainToolBar();

signals:
   void sig_showProject(int64_t prj_id);

public slots:
   void setCurrentClient(int64_t client_id);
   void setCurrentProject(int64_t prj_id);
   void showGUI();
   void userEventsChangeNotice();

private slots:
   void storeSettings();
   void systray_install();
   void finish_setup();
   void Record();
   void Pause();
   void Stop();
   void chargeOtherProject();
   void createClient();
   void editClient();
   void deleteClient();
   void relayCurrentProject(int ndx);
   void setMinVisDate();

   void client_update(int64_t);
   void client_remove(int64_t);

   void event_insert(int64_t);
   void event_update(int64_t);
   void event_remove(int64_t);

private:

   void syncControls();
   void sysTrayActivated(QSystemTrayIcon::ActivationReason reason);

   int64_t _visibleClient_id;

   QAction *_record_act,
          *_pause_act,
          *_stop_act,
          *_editClient_act,
          *_deleteClient_act,
          *_chargeOther_act,
          *_setMinVisDate_act;

   ProjectComboBox *_project_cb;

   int64_t _recording_prj_id;

   QSystemTrayIcon *_sysTray;
   QTimer *_trayTimer;
   QMenu *_trayMenu;
};

#endif // MAINTOOLBAR_H
