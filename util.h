#ifndef UTIL_H
#define UTIL_H

#include <QDebug>

#define J_DBG qDebug().nospace()  << __FILE__ << ":" << __LINE__ << " " 
#define J_DBG_FN J_DBG << __FUNCTION__ << "() "

const char*
sec2hrStr(int secs);
/***********************************************************
 * Convert seconds into an hour.decihour string.
 */

const char*
cent2dollarStr(int cents);
/***********************************************************
 * Convert cents into a dollar.cents string.
 */

#endif
