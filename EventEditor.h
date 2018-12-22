#ifndef EVENT_EDITOR_H
#define EVENT_EDITOR_H

#include <QWidget>

class QTreeWidget;

class EventEditor:public QWidget {

 Q_OBJECT
public:
   EventEditor(int64_t prj_id, QWidget *parent=0);
   ~EventEditor();

 private:
   QTreeWidget *_tw;
};

#endif				// EVENT_EDITOR_H
