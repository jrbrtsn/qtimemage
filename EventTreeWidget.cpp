
#include <assert.h>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "Event.h"
#include "EventTreeWidget.h"
#include "MainWindow.h"
#include "ProjectChooser.h"
#include "qtimemage.h"
#include "Report.h"
#include "util.h"

/*****************************************************************************/
/**************** EventTreeWidgetItem **************************************/
/*****************************************************************************/
const char* EventTreeWidgetItem::S_datetimeSec_fmt= "yyyy-MM-dd hh:mm:ss";
const char* EventTreeWidgetItem::S_datetimeMin_fmt= "yyyy-MM-dd hh:mm";


EventTreeWidgetItem::
EventTreeWidgetItem(int64_t event_id)
/*****************************************************************************
 * Constructor
 */
{
   Event *ev= G.eventTable[event_id];
   Q_ASSERT(ev);

   setData(0, Qt::UserRole, QVariant::fromValue(event_id));
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

   refresh();
}


EventTreeWidgetItem::
~EventTreeWidgetItem()
/*****************************************************************************
 * Destructor
 */
{
}

void
EventTreeWidgetItem::
setText(int column, const QString &text)
/*****************************************************************************
 * Set the text in a column of this item.
 */
{
   setData(column, Qt::DisplayRole, text);
}

int64_t
EventTreeWidgetItem::
event_id() const
/*****************************************************************************
 * Return the event_id for this item.
 */
{
   QVariant qv= data(0, Qt::UserRole);
   return qv.isValid() ? qv.toLongLong() : -1;
}

Event*
EventTreeWidgetItem::
getEvent() const
/*****************************************************************************
 * Return Event this item represents.
 */
{
   int64_t event_id= this->event_id();
   assert(-1 != event_id);
   return G.eventTable[event_id];
}

EventTreeWidgetItem*
EventTreeWidgetItem::
child(int ndx)
/*****************************************************************************
 * Return child at index cast to our type.
 */
{
   return static_cast<EventTreeWidgetItem*>(QTreeWidgetItem::child(ndx));
}

void
EventTreeWidgetItem::
refresh()
/*****************************************************************************
 * Refresh the text contents of this item.
 */
{
   Event *ev= getEvent();
   Q_ASSERT(ev);

   /* Set our columns of visible information */
   setText(TYPE_COL, ev->type_str());
   QDateTime qdt= QDateTime::fromSecsSinceEpoch(ev->when_ts());
   setText(TIMESTAMP_COL, qdt.toString(S_datetimeSec_fmt));
}

/*****************************************************************************/
/**************** EventTreeWidget ******************************************/
/*****************************************************************************/

EventTreeWidget::
EventTreeWidget(QWidget *parent)
/*****************************************************************************
 * Constructor
 */
 : QTreeWidget(parent)
 , _ignoreEditedEvent(false)
 , _menu(NULL)
{

   connect(&G.eventTable, SIGNAL(sig_insert(int64_t)), SLOT(event_insert(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_update(int64_t)), SLOT(event_update(int64_t)));
   connect(&G.eventTable, SIGNAL(sig_remove(int64_t)), SLOT(event_remove(int64_t)));

   setUniformRowHeights(true);
   setSelectionMode(QAbstractItemView::ContiguousSelection);

   QStringList headers;
   headers << "Event Type" << "Time Stamp";
   setColumnCount(EventTreeWidgetItem::N_COLS);
   setHeaderLabels(headers);
   setAlternatingRowColors(true);

   /* Field the result of an edit */
   connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(editedEvent(QTreeWidgetItem*,int)));

   /* Context menu */
   setContextMenuPolicy(Qt::CustomContextMenu);
   connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showEventCtxtMenu(const QPoint&)));

   setSortingEnabled(false);
}

EventTreeWidget::
~EventTreeWidget()
/*****************************************************************************
 * Destructor
 */
{}

EventTreeWidgetItem*
EventTreeWidget::
findItem(int64_t event_id)
/*****************************************************************************
 * Find an item by it's event_id.
 */
{
   EventTreeWidgetItem *parent= static_cast<EventTreeWidgetItem*>(invisibleRootItem());

   EventTreeWidgetItem *item;

   /* Search these children */
   for(int i= 0; i < parent->childCount(); ++i) {
      item= parent->child(i);
      if(event_id == item->event_id()) return item;
   }

   return NULL;
}

void
EventTreeWidget::
editedEvent(QTreeWidgetItem *item, int col)
/******************************************************************************************
 * An event got edited.
 */
{
   if(_ignoreEditedEvent) return;

   assert(item);

   EventTreeWidgetItem *etwItem= static_cast<EventTreeWidgetItem*>(item);

   Event *ev= etwItem->getEvent();
   Q_ASSERT(ev);

   switch(col) {
      case 0:
         {
           QString txt= etwItem->text(col);

           if(txt.contains("START", Qt::CaseInsensitive)) {

              ev->set_type_enum(Event::START);

           } else if(txt.contains("STOP", Qt::CaseInsensitive)) {

              ev->set_type_enum(Event::STOP);

           } else if(txt.contains("PAUSE", Qt::CaseInsensitive)) {

              ev->set_type_enum(Event::PAUSE);

           } else {

               QMessageBox::warning(this,
                                    "Invalid Input",
                                    "Sorry, must be one of START, STOP, PAUSE",
                                    QMessageBox::Ok);

               etwItem->refresh();
               /* Bail out early */
               return;
           }
         } break;

      case 1:
         {
           QString txt= etwItem->text(col);
           QDateTime ts= QDateTime::fromString(txt, EventTreeWidgetItem::S_datetimeSec_fmt);
           if(!ts.isValid())
              ts= QDateTime::fromString(txt, EventTreeWidgetItem::S_datetimeMin_fmt);

           if(!ts.isValid()) {
               QMessageBox::warning(this,
                                    "Invalid Input",
                                    QString("Sorry, must be in the form %1, or %2")
                                       .arg(EventTreeWidgetItem::S_datetimeMin_fmt)
                                       .arg(EventTreeWidgetItem::S_datetimeSec_fmt),
                                    QMessageBox::Ok);

               etwItem->refresh();
               /* Bail out early */
              return;
           }

           ev->set_when_ts(ts);
         } break;
   }

   /* If we get to here, input must have been valid, and ev contains new information. */
   ev->update();
}

EventTreeWidgetItem*
EventTreeWidget::
itemAt(const QPoint &p) const
/******************************************************************************************
 * Overload from QTreeWidget
 */
{
   QTreeWidgetItem *item= QTreeWidget::itemAt(p);
   return item ? static_cast<EventTreeWidgetItem*>(item) : NULL;
}

void
EventTreeWidget::
showEventCtxtMenu(const QPoint &pnt)
/******************************************************************************************
 * Show the view tree view context menu.
 */
{

   _slctn_lst= selectedItems();

   if(!_slctn_lst.count()) return;

   if(_menu) delete _menu;

   _menu= new QMenu(this);


   switch(_slctn_lst.count()) {

      case 1:
         switch(columnAt(pnt.x())) {

            case EventTreeWidgetItem::TYPE_COL:
               _menu->addAction("Edit Type", this, SLOT(editMenuEventType()));
               break;

            case EventTreeWidgetItem::TIMESTAMP_COL:
               _menu->addAction("Edit Timestamp", this, SLOT(editMenuEventTimestamp()));
               break;
         }

      default:
         _menu->addAction("Move Event to another project", this, SLOT(moveEvents()));
         break;
   }

   _menu->addAction("Delete Event(s)", this, SLOT(deleteMenuEvent()));
   
   _menu->popup(mapToGlobal(pnt));
}

void
EventTreeWidget::
setProject(int64_t project_id)
/*****************************************************************************
 * Populate the tree widget with projects.
 */
{
   EventTreeWidgetItem *item;
   int64_t event_id;

   _project_id= project_id;

   /* Make sure no items exist in the QTreeWidget */
   clear();

   if(-1 == project_id) return;

   QVector<int64_t> event_id_vec;
   int rtn = G.eventTable.fetchAll(event_id_vec, QString("WHERE project_id=%1 ORDER BY when_ts DESC").arg(project_id));
   if (rtn < 0) qFatal("Event::fetchAll() failed!");

   for (int i = 0; i < event_id_vec.size(); ++i) {

      event_id= event_id_vec[i];

      item= new EventTreeWidgetItem(event_id);

      addTopLevelItem(item);
   }
}

EventTreeWidgetItem*
EventTreeWidget::
slctnIndex2item(int ndx)
/******************************************************************************************
 * Convert a selection index into a usefully cat item.
 */
{
   QTreeWidgetItem *item= _slctn_lst[ndx];
   assert(item);
   return static_cast<EventTreeWidgetItem*>(item);
}

void
EventTreeWidget::
editMenuEventType()
/******************************************************************************************
 * Menu pick was made to edit event type.
 */
{
   editItem(slctnIndex2item(0), EventTreeWidgetItem::TYPE_COL);
}


void
EventTreeWidget::
editMenuEventTimestamp()
/******************************************************************************************
 * Menu pick was made to edit event timestamp.
 */
{
   editItem(slctnIndex2item(0), EventTreeWidgetItem::TIMESTAMP_COL);
}
   
void
EventTreeWidget::
deleteMenuEvent()
/******************************************************************************************
 * Menu pick was made to delete event.
 */
{
   int rtn = QMessageBox::warning(this,
		              "Delete Event",
			      "Are you sure you want to delete event(s)?",
			      QMessageBox::Ok | QMessageBox::Cancel,
			      QMessageBox::Cancel);

   if (rtn != QMessageBox::Ok) return;

   for(int i= 0; i < _slctn_lst.count(); ++i) {
      EventTreeWidgetItem *item= slctnIndex2item(i);
      Event *ev= G.eventTable[item->event_id()];

      Q_ASSERT(ev);

      if (ev->remove()) Q_ASSERT(0);
      // G.eventTable will emit a signal, and event_remove() will get called.
   }
}

void
EventTreeWidget::
moveEvents()
/************************************************************************************
 * An event record was inserted.
 */
{
   int rtn;
   /* Run the picker modally */
   ProjectChooser pc("Choose the Project to which you wish to move the selected events", G.pMw.data());
   rtn = pc.exec();

   if (rtn == QDialog::Accepted) {
      int64_t dstPrj_id= pc.selectedProject_id();

      for(int i= 0; i < _slctn_lst.count(); ++i) {
         EventTreeWidgetItem *item= slctnIndex2item(i);
         Event *ev= G.eventTable[item->event_id()];
         Q_ASSERT(ev);

         /* First make a copy of the old event, with the new
          * project_id.
          */
         Event newEvent;
         newEvent.set_project_id(dstPrj_id);
         newEvent.set_type_enum(ev->type_enum());
         newEvent.set_when_ts(ev->when_ts());

         /* Remove the old record */
         if (ev->remove()) Q_ASSERT(0);

         /* Insert the new record */
         if(newEvent.insert()) Q_ASSERT(0);

         // G.eventTable will emit a signal, and event_remove() will get called.
      }
   }

   G.userEventsChangeNotice();
}

void
EventTreeWidget::
event_insert(int64_t event_id)
/************************************************************************************
 * An event record was inserted.
 */
{
   Event *ev= G.eventTable[event_id];
   if(ev->project_id() != _project_id) return;

   EventTreeWidgetItem *item= new EventTreeWidgetItem(event_id);
   _ignoreEditedEvent= true;
   insertTopLevelItem(0, item);
   _ignoreEditedEvent= false;
}

void
EventTreeWidget::
event_update(int64_t event_id)
/************************************************************************************
 * An event record was updated.
 */
{
   Event *ev= G.eventTable[event_id];
   if(ev->project_id() != _project_id) return;

   EventTreeWidgetItem *item= findItem(event_id);
   Q_ASSERT(item);
   _ignoreEditedEvent= true;
   item->refresh();
   _ignoreEditedEvent= false;
}


void
EventTreeWidget::
event_remove(int64_t event_id)
/************************************************************************************
 * An event record was removed.
 */
{
   Event *ev= G.eventTable[event_id];
   if(ev->project_id() != _project_id) return;

   EventTreeWidgetItem *item= findItem(event_id);
   Q_ASSERT(item);
   int ndx= indexOfTopLevelItem(item);
   Q_ASSERT(ndx != -1);
   delete takeTopLevelItem(ndx);
}

