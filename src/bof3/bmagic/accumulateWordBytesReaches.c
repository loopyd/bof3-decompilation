#include "bof3/bof3.h"

/* This overlay's work-byte clear routine, already an exact lift of this target
 * at 0x801EFE9C: the jal at 0x801EF684 reaches it with no argument and no
 * return value consumed. */
void clearWorkBytesBmagicMagic05803_801EFE9C(void);

/* Shared frame counter in main RAM, outside this overlay's image, which this
 * target's generated undefined_syms_auto.txt already binds. Bit 0 is the
 * per-frame phase the bmagic lifts read as an odd-frame gate. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Battle record pointer published by the main executable, outside this overlay
 * image and already bound by this target's generated undefined_syms_auto.txt,
 * so no target-map row is needed. The word at 0x0B of the record it names is
 * the count the bmagic family's global-work-byte decrements walk down. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map and read once here, so every work-record access below is an offset of one
 * loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF61C
 * @behavior Final handler of the ring-ramp group of this overlay: word 8 of the
 *           nine-word handler run the payload header holds at 0x801EEC04 and
 *           entry 2 of the three-word sub-run at 0x801EEC1C that the frame
 *           dispatcher 0x801EF3B0 indexes with work byte 0x02, so no jal in this
 *           payload reaches it. It reads the scratchpad work pointer cell at
 *           0x1F800044 once and adds the work word at 0x18 to the word at 0x0C,
 *           the effect radius the draw handlers of this overlay read, then walks
 *           the radius on while the low bit of the shared frame counter at
 *           0x80143E6C is set, which is one frame of each pair: it decrements
 *           the work word at 0x18, the per-frame growth step, then the work byte
 *           at 0x0A, which the 0x801EFA78 draw handler scales into its vertex
 *           tint, and the growth-step store fills the branch delay slot while
 *           the tint byte's own decremented value feeds the zero test. When the
 *           tint byte reaches zero through that decrement the group ends: it reads
 *           the shared battle record pointer at 0x801EB4E0 once, decrements that
 *           record's byte 0x0B, and calls the clear routine at 0x801EFE9C with
 *           the byte store filling the call's delay slot, which zeroes work bytes
 *           0x00 through 0x04 so the frame dispatcher sees the work object as
 *           inactive. It keeps the return address at 0x10 in a 0x18-byte frame,
 *           takes no arguments, returns nothing and reads no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void accumulateWordBytesReaches(void) {
  u8 *work;
  u8 tint;
  u32 parity;

  work = g_battle_work;
  parity = D_80143E6C;
  *(u32 *)(work + 0x0C) += *(u32 *)(work + 0x18);
  if (parity & 1) {
    --*(u32 *)(work + 0x18);
    tint = --work[0x0A];
    if (tint == 0) {
      D_801EB4E0[0x0B]--;
      clearWorkBytesBmagicMagic05803_801EFE9C();
    }
  }
}
