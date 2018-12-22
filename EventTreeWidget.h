
#ifndef EVENT_TREE_WIDGET
#define EVENT_TREE_WIDGET

#include <QTreeWidget>
#include <QTreeWidgetItem>

class QMenu;
class Event;


/**************************************************************************
 * This wrapper class takes care of the tedium of the generic MVC nonsense,
 * as well as QVariants.
 **************************************************************************/
class EventTreeWidgetItem : public QTreeWidgetItem {

public:
   static const char *S_datetimeSec_fmt,
                     *S_datetimeMin_fmt;

   enum {
      TYPE_COL,
      TIMESTAMP_COL,
      N_COLS
   };

   EventTreeWidgetItem(int64_t prj_id);
   ~EventTreeWidgetItem();

  /* Eliminate default copy constructor and assignment operator */
   EventTreeWidgetItem(const EventTreeWidgetItem&)= delete;
   EventTreeWidgetItem& operator=(const EventTreeWidgetItem&)= delete;

   EventTreeWidgetItem* child(int ndx);

   Event* getEvent() const;

   void refresh();

   int64_t event_id() const;
   void setText(int column, const QString &text);
};

/**************************************************************************
 * Wrap the QTreeWidget class to customize for our needs.
 **************************************************************************/
class EventTreeWidget : public QTreeWidget {

   Q_OBJECT

public:

   EventTreeWidget(QWidget *parent);
   ~EventTreeWidget();

  /* Eliminate default copy constructor and assignment operator */
   EventTreeWidget(const EventTreeWidget&)= delete;
   EventTreeWidget& operator=(const EventTreeWidget&)= delete;

   EventTreeWidgetItem* itemAt(const QPoint &p) const;
   EventTreeWidgetItem* findItem(int64_t event_id);

   void setProject(int64_t prj_id);

signals:


private slots:
   void moveEvents();
   void showEventCtxtMenu(const QPoint&);
   void editedEvent(QTreeWidgetItem *item, int col);
   void editMenuEventType();
   void editMenuEventTimestamp();
   void deleteMenuEvent();
   void event_insert(int64_t);
   void event_update(int64_t);
   void event_remove(int64_t);

private:

   EventTreeWidgetItem* slctnIndex2item(int ndx);

   int64_t _project_id;
   bool _ignoreEditedEvent;
   QList<QTreeWidgetItem*> _slctn_lst;
   QMenu *_menu;
};

#endif
