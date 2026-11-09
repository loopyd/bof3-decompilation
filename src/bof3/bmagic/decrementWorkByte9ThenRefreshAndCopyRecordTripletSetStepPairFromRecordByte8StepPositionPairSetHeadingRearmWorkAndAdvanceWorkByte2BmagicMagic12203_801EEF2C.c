#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original re-materialises the cell with its own lui 0x1F80
 * plus lw 0x44($vN) pair at every read that follows a store through the loaded
 * pointer, because such a store may alias the cell, so the cell is named rather
 * than written as a bare scratchpad constant. The exact siblings of this target
 * decrementWorkByteBIncrementByte9RaiseFlag10At18ThenAdvanceWorkByte2Bmagic
 * Magic12203_801EF1C4 and
 * incrementWorkByte9AndAdvanceWorkByte2At78BmagicMagic12203_801EF24C declare it
 * the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image. This
 * target's generated undefined_syms_auto.txt binds the address because the
 * remaining asm spans func_801EEEA0 and func_801EF158 still load it. Byte 8 and
 * words 0x34, 0x38 and 0x3C read here are the fields the exact bmagic sibling
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC copies out of the same record, so
 * the record layout is shared and the address is not this target's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Acting battler byte at 0x80146374 in main RAM, outside this overlay image.
 * An index below 3 selects the party side, the test the exact bmagic sibling
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC makes on the same cell and the
 * one this body makes before choosing between the two step-pair paths. The
 * address keeps its generated undefined_syms_auto.txt row because the remaining
 * asm spans func_801EEEA0 and func_801EF158 still load it. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Two no-argument overlay helpers the original calls back to back with nothing
 * set up by this body, the form the exact bmagic sibling
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC calls func_800AF764 with. Both
 * are bound by this target's generated undefined_funcs_auto.txt rows. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */
extern void func_800AF764(void); /* @source 0x800AF764 */

/* Party-side overlay helper, reached with no argument set up by this body. */
extern void func_800B0118(void); /* @source 0x800B0118 */

/* Two-argument angle helper: the exact bmagic sibling
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC calls it with a pair of
 * differences and stores the answer as a heading word. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* One-argument overlay helper; every caller in this tree passes an encoded
 * constant, here 0x100. Bound by this target's generated
 * undefined_funcs_auto.txt row. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EEF2C
 * @behavior Frame step of the overlay work object reached through the
 *           scratchpad pointer cell 0x1F800044. It decrements the work
 *           object's unsigned byte 9 in place (lbu / addiu -1 / sb / andi 0xFF /
 *           bnez), testing the stored byte truncated to eight bits rather than a
 *           reload, so the rest of the body runs only when that countdown
 *           reaches zero. The first cell load precedes frame creation because
 *           the decrement needs no stack; the 0x18-byte frame keeps only $ra at
 *           0x10 across the four calls, so every base is re-read from the cell
 *           after a call or a store.
 *           On zero it copies the shared battle record's byte 8 at 0x801EB4E0
 *           into work byte 8, then calls the two no-argument overlay helpers
 *           func_800AFD9C and func_800AF764. Next it publishes the work object's
 *           halfwords 0x2E and 0x30, its projected position pair, into the
 *           scratchpad halfword cells 0x1F80000C and 0x1F80000E as the previous
 *           position, copies the record's words 0x34, 0x38 and 0x3C into the
 *           same offsets of the work object as the current position triplet, and
 *           calls func_800AF764 again.
 *           The acting battler byte at 0x80146374 then selects the step source:
 *           below 3 (the party side) it runs func_800B0118 and leaves the step
 *           pair alone; otherwise a switch on the work byte 8 just copied from
 *           the record writes the signed step pair 0xC / 0x10 as 0x29 with -0xD,
 *           -0x29 with -0xD, -0x26 with -0x26 for 2, and 0x26 with -0x26 for every
 *           other value. The pair is then added to the position pair in place at
 *           halfword width, the updated pair is published to the scratchpad
 *           halfword cells 0x1F800008 and 0x1F80000A, and func_8017A138 of the
 *           previous-minus-current differences of both axes gives the heading
 *           stored as work word 0x14.
 *           Last it re-arms the work object: byte 4 to 0x10, byte 0xB to 0x11,
 *           byte 9 to zero, byte 0xA to 1 and byte 2 incremented to advance the
 *           work step, and calls func_8015D71C with 0x100, the byte-2
 *           increment filling that call's delay slot. It takes no arguments,
 *           returns nothing and reads or writes no other state. The roles of
 *           work bytes 2, 4, 8, 9, 0xA and 0xB and of words 0xC, 0x10, 0x14,
 *           0x2E, 0x30, 0x34, 0x38 and 0x3C are not established beyond what this
 *           body does to them, so they stay offset-anchored and the step-pair
 *           constants are written as the literals they are.
 * @status partial
 * @match 91.37
 * @residual The position-pair update: the original re-reads work halfwords 0x2E
 *           and 0x30 from memory after the in-place add (lhu v0,46($v1) at
 *           0x801EF094 and lhu a1,48($v1) at 0x801EF0B8) and then sign-extends
 *           those reloads with sll/sra. Every tested clean-C spelling of the
 *           same statements either lets cse forward the value just stored into
 *           the halfword, so the reload disappears, or folds the read pair to a
 *           signed lh. Retained candidate: 127 of 139 instructions, 552 bytes
 *           against the original 556, first byte difference at +0x24 because the
 *           shortened body shifts the forward branch targets; the residual is
 *           inside the single position-update block, the whole rest of the body
 *           matches instruction for instruction.
 */
void decrementWorkByte9ThenRefreshAndCopyRecordTripletSetStepPairFromRecordByte8StepPositionPairSetHeadingRearmWorkAndAdvanceWorkByte2BmagicMagic12203_801EEF2C(void) {
  u8 *record;
  u8 *work;
  u8 *step;
  u8 *motion;
  u16 newX;
  u16 newY;
  s32 heading;

  if (--g_battle_work[9] == 0) {
    g_battle_work[8] = D_801EB4E0[8];
    func_800AFD9C();
    func_800AF764();
    work = g_battle_work;
    record = D_801EB4E0;
    *(u16 *)0x1F80000C = *(u16 *)(work + 0x2E);
    *(u16 *)0x1F80000E = *(u16 *)(work + 0x30);
    *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
    *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
    *(u32 *)(work + 0x3C) = *(u32 *)(record + 0x3C);
    func_800AF764();
    if (D_80146374 < 3) {
      func_800B0118();
    } else {
    step = g_battle_work;
      switch (step[8]) {
      case 0:
        *(s32 *)(step + 0xC) = 0x29;
        *(s32 *)(step + 0x10) = -0xD;
        break;
      case 1:
        *(s32 *)(step + 0xC) = -0x29;
        *(s32 *)(step + 0x10) = -0xD;
        break;
      case 2:
        *(s32 *)(step + 0xC) = -0x26;
        *(s32 *)(step + 0x10) = -0x26;
        break;
      default:
        *(s32 *)(g_battle_work + 0xC) = 0x26;
        *(s32 *)(g_battle_work + 0x10) = -0x26;
        break;
      }
    }
    motion = g_battle_work;
    *(u16 *)(motion + 0x2E) += *(s32 *)(motion + 0xC);
    newX = *(u16 *)(motion + 0x2E);
    *(u16 *)0x1F800008 = newX;
    *(u16 *)(motion + 0x30) += *(s32 *)(motion + 0x10);
    newY = *(u16 *)(motion + 0x30);
    *(u16 *)0x1F80000A = newY;
    heading = func_8017A138(*(s16 *)0x1F80000C - (s16)newX,
                            *(s16 *)0x1F80000E - (s16)newY);
    motion = g_battle_work;
    *(s32 *)(motion + 0x14) = heading;
    motion[4] = 0x10;
    g_battle_work[0xB] = 0x11;
    g_battle_work[9] = 0;
    g_battle_work[0xA] = 1;
    g_battle_work[2] += 1;
    func_8015D71C(0x100);
  }
}
