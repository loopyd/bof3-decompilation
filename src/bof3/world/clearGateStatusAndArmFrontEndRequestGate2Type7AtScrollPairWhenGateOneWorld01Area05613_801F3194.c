#include "bof3/bof3.h"

/* Shared gate-status halfword and gate byte of the area056/13 gate family; the
 * sibling entries 0x801F30A4-0x801F316C write 9/10/11/0xD/0xE/0xFFFF through
 * the same pair, where 0xFFFF is the "no gate" value. */
extern s8 D_801490C7;
extern u16 D_801490A8;

/* The three leading bytes of the shared front-end request record at
 * 0x80145E90: gate byte, type byte and selector byte. */
extern u8 D_80145E91; /* @source 0x80145E91 @kind unknown */
extern u8 D_80145E92; /* @source 0x80145E92 @kind unknown */
extern u8 D_80145E93; /* @source 0x80145E93 @kind unknown */

/* Shared scroll pair the world overlays read as row and column. */
extern s16 D_80145EC6[]; /* @source 0x80145EC6 @kind data */
extern s16 D_80145ECA;   /* @source 0x80145ECA @kind data */

/* Resident two-argument front-end request routine. */
void func_8015507C(s32 arg0, s32 arg1);

/* @source 0x801F3194
 * @behavior Entry 13 of the area056/13 overlay step table at 0x801F3804. It
 *           stores the no-gate sentinel 0xFFFF into the shared gate-status
 *           halfword D_801490A8 and then tests the shared gate byte
 *           D_801490C7: while that byte equals 1 it publishes a front-end
 *           request, storing gate byte 2 into D_80145E91, type byte 7 into
 *           D_80145E92 and selector byte 0 into D_80145E93 and calling
 *           func_8015507C over the shared scroll pair D_80145EC6/D_80145ECA.
 *           Every other gate value only leaves the sentinel behind. The pair
 *           reads precede the three record stores in the original, so the call
 *           arguments reach $a0/$a1 ahead of the store group. The gate byte
 *           load is hoisted above the status store, as in the sibling entry
 *           0x801F2FFC. The 0x18-byte frame only keeps $ra at 0x10. Takes no
 *           arguments and returns nothing; no caller reads the value left in
 *           $v0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearGateStatusAndArmFrontEndRequestGate2Type7AtScrollPairWhenGateOneWorld01Area05613_801F3194(
    void) {
  s32 cueArg;
  s32 cueId;

  D_801490A8 = 0xFFFF;
  if (D_801490C7 == 1) {
    cueArg = D_80145EC6[0];
    cueId = D_80145ECA;
    D_80145E91 = 2;
    D_80145E92 = 7;
    D_80145E93 = 0;
    func_8015507C(cueArg, cueId);
  }
}
