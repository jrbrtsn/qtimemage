#ifndef CLIENT_EDITOR_H
#define CLIENT_EDITOR_H

#include <QDialog>

class QLineEdit;
class Client;
class RateEditor;
class QSpinBox;

class ClientEditor:public QDialog {

 Q_OBJECT
public:
   ClientEditor(QWidget *parent, const Client *src);
   ~ClientEditor();

   void assign(Client *target) const;

 private:
   QLineEdit *_acronym_le, *_name_le;
   RateEditor *_default_rate_re;
   QSpinBox *_charge_quantum_sb;
};

#endif				// CLIENT_EDITOR_H
