#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class QTabWidget;
class ClientTabBar;
class Client;
class ClientPage;
class MainToolBar;
class QSystemTrayIcon;

class MainWindow:public QMainWindow {

   Q_OBJECT
public:
   MainWindow(QWidget * parent = 0);
   ~MainWindow();

signals:
   void sig_currentClient(int64_t client_id);

public slots:
   void showProject(int64_t prj_id);

private slots:
   void show_errMsg(const QString& msg);
   void storeSettings();
   void currentClientChanged(int ndx);
   void populate_clients();
   void clientTabMoved();

   void client_insert(int64_t);
   void client_update(int64_t);
   void client_remove(int64_t);

   void event_insert(int64_t);
   void event_update(int64_t);
   void event_remove(int64_t);

protected:
   void closeEvent(QCloseEvent *event) override;

private:

   int findClientNdx(int64_t client_id);
   int findProjectClientNdx(int64_t prj_id);
   int64_t currentClient_id();

   struct {
      ClientTabBar *tb;
      ClientPage *pg;
   } _client;
  
   MainToolBar *_main_tb;

};

#endif				// MAINWINDOW_H
