#ifndef EMI_WORLD00_AREA030_05_INTERNAL_H
#define EMI_WORLD00_AREA030_05_INTERNAL_H

#include "bof3/bof3.h"

extern u8 handlerIndex; /* @source 0x800F724C @kind data */
extern void (*handlerTable[])(void); /* @source 0x800F71F0 @kind table */

void dispatchArea030CompanionHandler(void); /* @source 0x800F500C */

#endif
