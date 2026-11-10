#include "bof3/bof3.h"

/* Shared gate byte and gate-status halfword of the area056/13 gate family; the
 * sibling entries 0x801F30A4-0x801F316C write 9/10/11/0xD/0xE/0xFFFF through
 * the same pair, where 0xFFFF is the "no gate" value. */
extern s8 D_801490C7;
extern u16 D_801490A8;

/* Shared flag bank the sibling entry 0x801F2C04 also submits to
 * func_8015B580, and the three leading bytes of the shared front-end request
 * record at 0x80145E90: gate byte, type byte and selector byte. */
extern u8 D_80144EA0; /* @source 0x80144EA0 @kind unknown */
extern u8 D_80145E91; /* @source 0x80145E91 @kind unknown */
extern u8 D_80145E92; /* @source 0x80145E92 @kind unknown */
extern u8 D_80145E93; /* @source 0x80145E93 @kind unknown */

/* Shared scroll pair the world overlays read as row and column. */
extern s16 D_80145EC6[]; /* @source 0x80145EC6 @kind data */
extern s16 D_80145ECA;   /* @source 0x80145ECA @kind data */

/* Resident front-end cue routine, the flag-bit helper and the cue request. */
void func_8015DF18(u16 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015507C(s32 arg0, s32 arg1);

/* @source 0x801F2FFC
 * @behavior One step of the area056/13 overlay step table at 0x801F3804. It
 *           first stores the no-gate sentinel 0xFFFF into the shared
 *           gate-status halfword D_801490A8, then dispatches on the shared
 *           gate byte D_801490C7, which it re-reads for each test: a zero gate
 *           publishes the shared front-end request, arming gate byte
 *           D_80145E91 with 2, type byte D_80145E92 with 7 and selector byte
 *           D_80145E93 with 0 and then calling func_8015507C over the shared
 *           scroll pair D_80145EC6/D_80145ECA; a gate of 1 submits the shared
 *           flag bank D_80144EA0 bit 0x0E through func_8015B580; and any gate
 *           other than 2 queues front-end cue 0x208 through func_8015DF18.
 *           The pair reads precede the three record stores in the original, so
 *           the call arguments reach $a0/$a1 ahead of the store group. The
 *           0x18-byte frame only keeps $ra at 0x10. Takes no arguments and
 *           returns nothing; no caller reads the value left in $v0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armFrontEndRequestWhenGateClearSetFlagBankEWhenGateOneQueueCue208UnlessGateTwoWorld01Area05613_801F2FFC(
    void) {
  s32 cueArg;
  s32 cueId;

  D_801490A8 = 0xFFFF;
  if (D_801490C7 == 0) {
    cueArg = D_80145EC6[0];
    cueId = D_80145ECA;
    D_80145E91 = 2;
    D_80145E92 = 7;
    D_80145E93 = 0;
    func_8015507C(cueArg, cueId);
  }
  if (D_801490C7 == 1) {
    func_8015B580((u32)&D_80144EA0, 0xE);
  }
  if (D_801490C7 != 2) {
    func_8015DF18(0x208);
  }
}
