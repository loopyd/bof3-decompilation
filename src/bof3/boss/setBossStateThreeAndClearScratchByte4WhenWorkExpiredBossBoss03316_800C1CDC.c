#include "bof3/bof3.h"

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad work-record pointer cell: the battle code publishes the work
 * record pointer here and the record's bytes are reached through it. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared boss state byte. */
extern u8 D_80146375; /* @source 0x80146375 @kind unknown */

/* @source 0x800C1CDC
 * @behavior Entry 0 of the per-index handler table based at 0x800C27A0, which
 * the work-step handler func_800C1CA4 indexes with the low byte of its argument
 * (scaled by four) and calls; the same word is entry 17 of the code-pointer run
 * that starts at 0x800C275C. It takes no arguments and returns nothing. It
 * loads the current enemy work record pointer published at 0x801EB4E8 and reads
 * that record's halfword at byte offset 0x94, the field the sibling at
 * 0x800C1D14 overwrites with 0xFFFF once its remaining-versus-count test is
 * met. While that halfword equals 0xFFFF it stores the constant 3 into the
 * shared boss state byte 0x80146375 and clears byte 4 of the scratch work
 * record reached through the pointer cell 0x1F800044; otherwise it returns
 * without touching either. The original issues the 0x1F800044 load before the
 * state byte store, so the source names one local for the record pointer and
 * lets the scratch cell be read once before that store. The role of state value
 * 3 and of the scratch byte at offset 4 are not established here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setBossStateThreeAndClearScratchByte4WhenWorkExpiredBossBoss03316_800C1CDC(
    void) {
  u8 *work;

  work = D_801EB4E8;
  if (FIELD_REF(u16, work, 0x94u) == 0xFFFFu) {
    D_80146375 = 3;
    D_1F800044[4] = 0;
  }
}
