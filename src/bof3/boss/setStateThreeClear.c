#include "bof3/bof3.h"

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad work-record pointer cell: the battle code publishes the work
 * record pointer here and the record's bytes are reached through it. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle state byte the boss overlays store their state constant into. */
extern u8 D_80146375; /* @source 0x80146375 @kind unknown */

/* Main-executable helper at 0x8015D71C, invoked here with the selector 0x602.
 * Its answer is dead at this site - the body returns without reading it - so the
 * declaration carries no return value. Splat binds 0x8015D71C through this
 * target's generated undefined_funcs_auto.txt, so no target-map row is needed. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x800C1A00
 * @behavior Entry 2 of the twelve-word handler run based at 0x800C1FEC, the
 * word 0x800C1FF4 of the payload data row D_800C1F9C: the dispatcher
 * dispatchStateByte2Table1FECBossBoss00216_800C18A0 indexes that run from
 * work-record byte 2 and calls the selected entry with no arguments, and no jal
 * in the payload names this body, so its parameters stay structural. It loads
 * the current enemy work record pointer published at 0x801EB4E8 ahead of its own
 * frame setup and reads that record's halfword at 0x94, the field the
 * neighboring entry 0x800C1A4C overwrites with 0xFFFF once the work is expired.
 * While that halfword equals 0xFFFF it reads the scratchpad pointer cell
 * 0x1F800044, stores the constant 3 into the shared battle state byte
 * 0x80146375, clears byte 4 of the scratch work record and invokes the shared
 * main-executable helper 0x8015D71C with the selector 0x602; otherwise it
 * returns without touching any state. The 0x18-byte frame only keeps $ra at
 * 0x10 across that call. The roles of state value 3 and of the scratch byte at
 * offset 4 are not established here, so they stay named by offset. The
 * statement order and the local spelling come from the exact sibling
 * setBossStateThreeAndClearScratchByte4WhenWorkExpiredBossBoss03316_800C1CDC,
 * whose 0x38-byte body was the same fourteen leading words of this span without
 * the selector-0x602 call and the frame that call needs. It takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStateThreeClear(
    void) {
  u8 *work;
  u8 *spad;

  work = D_801EB4E8;
  if (FIELD_REF(u16, work, 0x94u) == 0xFFFFu) {
    spad = D_1F800044;
    D_80146375 = 3;
    spad[4] = 0;
    func_8015D71C(0x602);
  }
}
