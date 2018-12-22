#include "qtimemage.h"
#include "configItem.h"

/**************************************************************************************/
/********************************* configItem *****************************************/
/**************************************************************************************/

configItem::
configItem(const char *key)
/*******************************************************
 * Insert ourselves into the global registry.
 */
: _key(key)
{
  G.register_configItem(key, this);
}

/**************************************************************************************/
/******************************** configSize ******************************************/
/**************************************************************************************/
configSize::
configSize(const char *key, int width, int height)
/*******************************************************
 * Prepare for use.
 */
: configItem(key)
{
  setWidth(width);
  setHeight(height);
}

int
configSize::
parse_line(const QString &line)
/*******************************************************
 * Keep a copy of the line.
 */
{
  int width, height, rtn= 1;
  if(sscanf(line.toLatin1().data(), "%d %d", &width, &height) != 2) goto abort;
  setWidth(width);
  setHeight(height);
  rtn= 0;

abort:
  return rtn;
}

QString
configSize::
write_line() const
/*******************************************************
 * Write to output
 */
{
  QString rtn;
  rtn.sprintf("%d %d", width(), height());
  return rtn;
}

bool
configSize::
has_info() const
/*******************************************************
 * Whether or not there is anything to write with write_line.
 */
{
  return true;
}

/**************************************************************************************/
/******************************** configRect ******************************************/
/**************************************************************************************/
configRect::
configRect(const char *key, int x, int y, int width, int height)
/*******************************************************
 * Prepare for use.
 */
: configItem(key)
{
  setRect(x, y, width, height);
}

int
configRect::
parse_line(const QString &line)
/*******************************************************
 * Keep a copy of the line.
 */
{
  int x, y, width, height, rtn= 1;
  if(sscanf(line.toLatin1().data(), "%d %d %d %d", &x, &y, &width, &height) != 4) goto abort;
  setRect(x, y, width, height);
  rtn= 0;

abort:
  return rtn;
}

QString
configRect::
write_line() const
/*******************************************************
 * Write to output
 */
{
  QString rtn;
  rtn.sprintf("%d %d %d %d", x(), y(), width(), height());
  return rtn;
}

bool
configRect::
has_info() const
/*******************************************************
 * Whether or not there is anything to write with write_line.
 */
{
  return true;
}

/**************************************************************************************/
/******************************** configInt ****************************************/
/**************************************************************************************/

configInt::
configInt(const char *key, int val)
/*******************************************************
 * Prepare for use.
 */
: configItem(key)
, _val(val)
{ }

int
configInt::
parse_line(const QString &line)
/*******************************************************
 * Keep a copy of the line.
 */
{
  bool tf; 
  _val= line.toInt(&tf);
  return tf ? 0 : 1;
}

QString
configInt::
write_line() const
/*******************************************************
 * Write to output
 */
{
  return QString::number(_val);
}

bool
configInt::
has_info() const
/*******************************************************
 * Whether or not there is anything to write with write_line.
 */
{
  return true;
}
/**************************************************************************************/
/******************************** configString ****************************************/
/**************************************************************************************/

configString::
configString(const char *key)
/*******************************************************
 * Prepare for use.
 */
: configItem(key)
{ }

int
configString::
parse_line(const QString &line)
/*******************************************************
 * Keep a copy of the line.
 */
{
  *this= line;
  return 0;
}

QString
configString::
write_line() const
/*******************************************************
 * Write to output
 */
{
  return trimmed();
}

bool
configString::
has_info() const
/*******************************************************
 * Whether or not there is anything to write with write_line.
 */
{
  return size() ? true : false;
}

/**************************************************************************************/
/********************************* configIntVec ********************************/
/**************************************************************************************/
configIntVec::
configIntVec(const char *key)
/*******************************************************
 * Prepare for use.
 */
: configItem(key)
{}


int
configIntVec::
parse_line(const QString &line)
/*******************************************************
 * Parse line into integer vector.
 */
{
  int rtn= 1;
  QStringList str_lst= line.split(",");

  for(int section= 0; section < str_lst.size(); section++) {
    bool ok;
    int sz= str_lst[section].toInt(&ok);
    if(!ok) goto abort;
    append(sz);
  }
  rtn= 0;

abort:
  return rtn;
}

QString
configIntVec::
write_line() const
/*******************************************************
 * Write to output
 */
{
  QString rtn;
  for(int i= 0; i < size(); i++) {
    if(i) rtn += ",";
    rtn += QString::number(at(i));
  }
  return rtn;
}

bool
configIntVec::
has_info() const
/*******************************************************
 * Whether or not there is anything to write with write_line.
 */
{
  return size() ? true : false;
}

