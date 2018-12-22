#include <QLineEdit>

class RateEditor : public QLineEdit {

   Q_OBJECT

public:
   RateEditor(unsigned int pennies, QWidget *parent=0);
   ~RateEditor();

   unsigned int pennies() const;

};
