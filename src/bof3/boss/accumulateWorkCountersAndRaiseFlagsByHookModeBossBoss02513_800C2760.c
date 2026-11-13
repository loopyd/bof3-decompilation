#include "bof3/bof3.h"

/* Shared main-program state this dispatcher tests. 0x80146374 is the battle
 * phase byte the case-0 arm compares with 3, and 0x80146384 is the byte that
 * arm indexes and then re-tests. 0x80145FAC is the signed halfword at +0x11C of
 * the 0x140-stride local work run (base 0x80145E90) that the same byte selects,
 * so the base is declared as a byte span and the stride is written into the
 * index. 0x801EB728 is a signed halfword operand of the second work counter and
 * 0x801EB730 the word whose 0x2 bit chooses between that counter and the byte
 * counter. The 0x801EB6xx bytes are the enemy work record fields the case-1 arm
 * stamps, and 0x80146322 is the ranked-owner cursor that arm gates on. 0x801463D0
 * is the word the case-2 arm compares with 0x15, and 0x80146328 the flag byte it
 * raises bit 0x4 in. 0x801462ED and 0x801462E0 are the flag byte and mode byte of
 * the case-3 arm. */
extern u8 D_80146374;     /* @source 0x80146374 @kind unknown */
extern u8 D_80146384;     /* @source 0x80146384 @kind unknown */
extern u8 D_80145FAC[];   /* @source 0x80145FAC @kind table */
extern s16 D_801EB728;    /* @source 0x801EB728 @kind unknown */
extern u32 D_801EB730;    /* @source 0x801EB730 @kind unknown */
extern u8 D_801EB631;     /* @source 0x801EB631 @kind unknown */
extern u8 D_801EB632;     /* @source 0x801EB632 @kind unknown */
extern u8 D_801EB725;     /* @source 0x801EB725 @kind unknown */
extern u8 D_801EB6AE;     /* @source 0x801EB6AE @kind unknown */
extern u8 D_80146322;     /* @source 0x80146322 @kind unknown */
extern u32 D_801463D0;    /* @source 0x801463D0 @kind unknown */
extern u8 D_80146328;     /* @source 0x80146328 @kind unknown */
extern u8 D_801462ED;     /* @source 0x801462ED @kind unknown */
extern u8 D_801462E0;     /* @source 0x801462E0 @kind unknown */

/* Three work counters of this overlay. The sibling installer 0x800C2710
 * published this dispatcher into the shared battle hook slot 0x801463AC and
 * cleared exactly these three cells. */
extern u32 D_801FE4F0; /* @source 0x801FE4F0 @kind unknown */
extern u32 D_801FE4E8; /* @source 0x801FE4E8 @kind unknown */
extern u8 D_801FE4EF;  /* @source 0x801FE4EF @kind unknown */

/* Action-slot allocator of the shared battle image at 0x801E590C, outside this
 * overlay and outside this payload, so it keeps its raw address name. */
u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x800C2760
 * @behavior Overlay hook-mode dispatcher: the third member of the handler trio
 *           the sibling installer 0x800C2710 publishes into the shared battle
 *           hook slot 0x801463AC. Masks the mode byte and switches through the
 *           six-entry jump table at 0x800C181C; modes 4 and 5 fall straight to
 *           the one return and every path returns zero.
 *           Mode 0 has two arms. While the battle phase byte 0x80146374 reads 3
 *           and the byte 0x80146384 is below 3, it adds the signed halfword at
 *           +0x11C of that byte's 0x140-stride work record (0x80145FAC) into the
 *           work counter 0x801FE4F0 and returns; otherwise it re-reads 0x80146384
 *           and, when that byte is 3 or holds any of the 0xC0 bits, raises the
 *           byte counter 0x801FE4EF while bit 0x2 of 0x801EB730 is set and
 *           otherwise adds the signed halfword 0x801EB728 into the work counter
 *           0x801FE4E8. Mode 1 runs only while the ranked-owner cursor 0x80146322
 *           reads 1 and the enemy record byte 0x801EB6AE reads the same value: it
 *           stamps 0x801EB631 and 0x801EB725 with 2, clears 0x801EB632 and raises
 *           bit 0x2 of 0x801EB730. Mode 2 raises bit 0x4 of the battle flag byte
 *           0x80146328 while 0x801463D0 reads 0x15. Mode 3 runs only while bit
 *           0x20 of 0x801462ED is clear and 0x801462E0 reads 1: it submits effect
 *           8 through the shared action-slot allocator func_801E590C and then
 *           raises that same 0x20 bit.
 *           Mode 0 reaches the one tail return with break at each exit rather
 *           than returning zero per site: a seed that returned zero at every
 *           exit measured 116 of 137 instructions at the payload's size, first
 *           difference on the register the 0x80145FAC halfword lands in, while
 *           breaking to the shared tail lets the compiler copy that tail's zero
 *           result into each branch's delay slot, which is where the original
 *           carries it. Mode 3's two early zero returns are unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 accumulateWorkCountersAndRaiseFlagsByHookModeBossBoss02513_800C2760(u8 mode) {
  u8 *phase_flags;

  switch (mode) {
  case 0:
    if (D_80146374 == 3) {
      if (D_80146384 < 3) {
        s16 gain = *(s16 *)(void *)(D_80145FAC + D_80146384 * 0x140);

        if (gain > 0) {
          D_801FE4F0 += gain;
          break;
        }
      }
    }
    if ((D_80146384 & 0xFF) == 3 || (D_80146384 & 0xC0) != 0) {
      if ((D_801EB730 & 0x2) == 0) {
        if (D_801EB728 > 0) {
          D_801FE4E8 += D_801EB728;
        }
        break;
      }
      D_801FE4EF++;
      break;
    }
    break;
  case 1:
    if (D_80146322 != 1) {
      break;
    }
    if (D_801EB6AE != D_80146322) {
      break;
    }
    D_801EB631 = 2;
    D_801EB632 = 0;
    D_801EB725 = 2;
    D_801EB730 |= 0x2;
    break;
  case 2:
    if (D_801463D0 == 0x15) {
      D_80146328 |= 0x4;
    }
    break;
  case 3:
    phase_flags = &D_801462ED;
    if ((*phase_flags & 0x20) != 0) {
      return 0;
    }
    if (D_801462E0 != 1) {
      return 0;
    }
    func_801E590C(0, 8);
    *phase_flags |= 0x20;
    break;
  case 4:
    break;
  case 5:
    break;
  }

  return 0;
}
