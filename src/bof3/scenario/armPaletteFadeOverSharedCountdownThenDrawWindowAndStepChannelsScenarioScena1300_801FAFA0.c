#include "bof3/bof3.h"

extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void clampPaletteChannelsScenarioScena1300_801FB0B8(u32 intensity);
void copyPaletteBlock32AndAdvanceSerialScenarioScena1300_801FB164(void);

/* Shared scenario countdown halfword that this overlay's states reload through
 * their own stores. */
extern u16 D_80146876;
/* Shared status halfword published by the world and scene controllers. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */
/* Scratchpad halfwords holding the fade step difference and the fade step
 * total of this overlay's palette ramp. */
extern u16 D_1F800000[]; /* @source 0x1F800000 @kind data */

/* @source 0x801FAFA0
 * @behavior Phases this overlay's palette fade against the shared scenario
 * countdown. It derives the total step count 0x1E * (frames & 0xFF) - 1 from
 * its second argument, stores it in the scratchpad halfword 0x1F800002,
 * publishes its first argument as the shared status halfword 0x801490A8, and
 * stores that total minus the shared countdown halfword 0x80146876 in the
 * scratchpad halfword 0x1F800000. While the stored difference compares below
 * the total in an unsigned halfword compare (that is, while the countdown lies
 * between one and the total) it first submits one 0x50 by 0x3C func_8014F800
 * window whose texture handle is the main-RAM halfword offset table 0x80010000
 * indexed by the first argument added to 0x80010002, and then steps the palette
 * window: a
 * difference below 0x20 clamps the palette channels with its own low byte, a
 * difference of exactly 0x20 copies the palette block, and a difference that
 * the fade total minus 0x20 compares below clamps the channels with 0x20 minus
 * the gap between that difference and the total minus 0x20. When the difference
 * does not compare below the total it copies the palette block only if the
 * difference still equals the total, so a countdown of zero restores the block
 * and a countdown beyond the total leaves the palette untouched. Every decision
 * after the window call re-reads the scratchpad halfwords, and the last channel
 * store is the final statement before the epilogue. The total is carried as a
 * 32-bit value and the difference is read back as a halfword, so both unsigned
 * compares widen it with `andi 0xFFFF`, while the fade-total read is compared
 * signed so a total below its own 0x20 bias stays negative. Takes the texture
 * id and the fade length as its two arguments and returns nothing. Evidence for
 * the shapes: the two stepping callees are the exact siblings
 * clampPaletteChannelsScenarioScena1300_801FB0B8 (0x801FB0B8) and
 * copyPaletteBlock32AndAdvanceSerialScenarioScena1300_801FB164 (0x801FB164),
 * whose recorded measurements name these two `jal` sites in this range, and the
 * 0x80010000 offset table is the table the sisyou00 header records as an
 * offset base for the func_8014F800 texture handle. A bounded search of the
 * sibling scenario overlays found no byte-identical copy of this body; the
 * scena16 controller walks the same countdown and palette pair with different
 * counts and no such setup block, so it supplies shape only.
 * @status partial
 * @match 82.86
 * @residual Reassociation and register order after the window call.
 *
 * Detail: the retained body reproduces 58 of the original 70 instructions in
 * 280 bytes, and every remaining difference sits in the block after the window
 * call. The code generator reassociates that branch's channel value as
 * 0x20 - ((remain + 0x20) - total) where the original computes remain minus the
 * total-biased-by-0x20 term first, and it keeps the reloaded fade difference,
 * its 0xFFFF-widened copy and the 0x20 limit in $v1/$a0/$a1 where the original
 * uses $a1/$v1/$a2. Shapes measured and rejected on this block: signed and
 * unsigned literal spellings of the same nesting, a signed inner term shared
 * with the branch test, an explicit u16 truncation, a u32 snapshot with an
 * explicit 0xFFFF mask at every test, and a statement-per-line rewrite through
 * an s32/u32 level local; the last two lost five instructions each and the
 * others kept this residual unchanged.
 */
void armPaletteFadeOverSharedCountdownThenDrawWindowAndStepChannelsScenarioScena1300_801FAFA0(
    u32 id, u32 frames) {
  u32 total;
  u32 limit;
  u16 remain;
  u16 *steps;

  steps = &D_1F800000[1];
  total = 0x1Eu * (frames & 0xFFu) - 1u;
  limit = total & 0xFFFFu;
  *steps = (u16)total;
  D_801490A8 = (u16)id;
  D_1F800000[0] = (u16)(total - D_80146876);

  if (D_1F800000[0] < limit) {
    func_8014F800(0x50, 0x3C, 0, 0xFFu,
                  0x80010002u + (u32)PSX_PTR(u16, 0x80010000u)[(u16)id]);
    remain = D_1F800000[0];
    if (remain < 0x20u) {
      clampPaletteChannelsScenarioScena1300_801FB0B8(remain & 0xFFu);
    } else if (remain == 0x20u) {
      copyPaletteBlock32AndAdvanceSerialScenarioScena1300_801FB164();
    } else if ((s32)(*steps - 0x20u) < (s32)remain) {
      clampPaletteChannelsScenarioScena1300_801FB0B8(
          (0x20 - (remain - (*steps - 0x20u))) & 0xFF);
    }
  } else if (D_1F800000[0] == limit) {
    copyPaletteBlock32AndAdvanceSerialScenarioScena1300_801FB164();
  }
}
