#ifndef EXE_LOGO_SYMBOLS_FUNCTIONS_H
#define EXE_LOGO_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

/* LOGO.EXE startup and CAPCOM30.STR scheduler. */
void   func_801CE758(void);
void   func_801CE760(s32 work_base, u_long disc_lba);
void   func_801CE7F4(void);
s32    func_801CE930(u_long disc_lba);
s32    func_801CED48(void);
s32    func_801CEA98(void);
void   func_801CEBFC(void);
void   func_801CEDFC(void);
s32    func_801CEE7C(s32 port);
u_long func_801CEECC(s32 port);
void   func_801CEEF4(void);
s32    func_801D209C(s32 arg0, s32 arg1);

#endif
