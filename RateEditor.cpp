#include "RateEditor.h"

RateEditor::
RateEditor(unsigned int pennies, QWidget *parent)
/*****************************************************************************
 * Constructor
 */
: QLineEdit(parent)
{
   int rate_dollars= pennies/100,
       rate_cents= pennies%100;
   QString str=QString("%1.%2").arg(rate_dollars).arg(rate_cents,2,10,QLatin1Char('0'));
   setText(str);
   setInputMask("0009.99");
}


RateEditor::
~RateEditor()
/*****************************************************************************
 * Destructor
 */
{
}

unsigned int
RateEditor::
pennies() const
/*****************************************************************************
 * Convert string back to pennies.
 */
{
   bool ok;
   double dval= text().toDouble(&ok);

   Q_ASSERT(ok);

   return dval*100;
}
