#ifndef EXE_LOGO_SYMBOLS_FUNCTIONS_H
#define EXE_LOGO_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

/* LOGO.EXE startup and CAPCOM30.STR scheduler. */
void   func_801ce758(void);
void   func_801ce760(void* work_base, u_long disc_lba);
s32    func_801cea98(void);
void   func_801cebfc(void);
void   func_801cedfc(void);
u_long func_801ceecc(s32 port);
void   func_801ceef4(void);

#endif
