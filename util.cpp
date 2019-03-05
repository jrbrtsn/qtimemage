#include "stdio.h"
#include "util.h"

const char*
decihour2hrStr(int decihours)
/***********************************************************
 * Convert seconds into an hour.decihour string.
 */
{
   static char buf[16];
   double hours= (double)decihours/10.;
   ::snprintf(buf, sizeof(buf-1), "%.1lf", hours);
   return buf;
}

const char*
cent2dollarStr(int cents)
/***********************************************************
 * Convert cents into a dollar.cents string.
 */
{
   static char buf[16];
   int d= cents/100,
       c= cents%100;
       
   ::snprintf(buf, sizeof(buf-1), "%d.%02d", d, c);
   return buf;
}

