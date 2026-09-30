#ifndef EXE_LOGO_SYMBOLS_FUNCTIONS_H
#define EXE_LOGO_SYMBOLS_FUNCTIONS_H

#include "bof3/bof3.h"

/* LOGO.EXE startup and CAPCOM30.STR scheduler. */
void func_801CE758(void);
void initWorkAreaAndStartSubsystems(s32 work_base, u_long disc_lba);
void func_801CE7F4(void);
s32  func_801CE930(u_long disc_lba);
void func_801CED48(void);
void func_801CEDA4(void);
s32  func_801CEA98(void);
void func_801CEBFC(void);
void playCapcomStream(void);
void func_801CEEF4(void);
void func_801CE8DC(void);

/* Uncatalogued functions inside the LOGO.EXE image called by the stream
 * teardown; declared by address only until their PsyQ object identity is
 * proven. func_801D0158 dispatches through the CD hook table and reports
 * completion, func_801CFEA4 and func_801D02BC are CD transfer helpers and
 * func_801D2294 resets the CD hook state. */
s32 func_801CFEA4(s32 mode, s32* result);
s32 func_801D0158(s32 mode, u8* flag, s32 arg);
s32 func_801D02BC(s32 buffer, s32 length);
void func_801D2294(void);

/* Uncatalogued functions inside the LOGO.EXE image used by the CAPCOM30.STR
 * frame pump func_801CEA98, again declared by address only. func_801D2558
 * publishes the VLC bitstream and frame-header pointers for the next stream
 * frame, func_801D2464 advances the decoder stream position, func_801D6FCC
 * submits the 0xB40-byte CD work buffer and func_801CEC88 advances the LOGO
 * frame column inside func_801CEBFC's boundary. */
s32  func_801D2558(u_long** data, u_long** header);

/* Uncatalogued functions inside the LOGO.EXE image used by the CAPCOM30.STR
 * stream setup func_801CE930, again declared by address only. func_801CECA4 is
 * the MDEC output callback installed for the stream, func_801CFC00 arms the CD
 * stream for the D_801EB448 buffer, func_801CFEF4 reports the readiness of a
 * stream flag through its second pointer argument, func_801D2180 waits for one
 * frame of the given byte size and func_801D23DC and func_801D0320 start the
 * disc transfer of the supplied LBA. */
s32  func_801CFEF4(s32 mode, u8* flag, s32 arg);
void func_801CFC00(s32 buffer, s32 mode);
s32  func_801D2180(s32 size);
s32  func_801D23DC(s32 arg0, s32 arg1, u_long arg2, s32 arg3, s32 arg4);
s32  func_801D0320(u_long lba);
void func_801CECA4(void);
void func_801D2464(u_long* data);
s32  func_801D6FCC(s32 buffer, s32 length);
void func_801CEC88(void);

/* Uncatalogued functions inside the LOGO.EXE image used by the MDEC output
 * callback func_801CECA4, again declared by address only. func_801D263C
 * publishes its incoming argument word into D_801EB678 and func_801D44D4
 * consumes the frame column RECT D_801EB45C with the D_801EB444 work buffer.
 * Both are called from the CD/GPU library region as well, so their PsyQ object
 * identity is unproven. */
void func_801D263C(void);
s32  func_801D44D4(RECT* rect, s32 buffer);

#endif
