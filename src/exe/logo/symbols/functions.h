#ifndef EXE_LOGO_SYMBOLS_FUNCTIONS_H
#define EXE_LOGO_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

/* LOGO.EXE startup and CAPCOM30.STR scheduler. */
void   func_801CE758(void);
void   func_801CE760(void* work_base, u_long disc_lba);
s32    func_801CEA98(void);
void   func_801CEBFC(void);
void   func_801CEDFC(void);
u_long func_801CEECC(s32 port);
void   func_801CEEF4(void);

#endif
