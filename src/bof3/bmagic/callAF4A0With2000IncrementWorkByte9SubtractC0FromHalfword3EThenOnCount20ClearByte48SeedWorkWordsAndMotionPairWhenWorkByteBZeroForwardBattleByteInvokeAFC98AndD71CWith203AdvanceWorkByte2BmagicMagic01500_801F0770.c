#include "bof3/bof3.h"

/* Shared battle record pointer cell in main RAM, outside this overlay's image.
 * Splat's generated undefined-syms map already binds the address, so the name
 * needs no target-map row. The sibling lift callAF4A0With1800DecrementWorkByte9
 * AddWorkWord20ToWord14AccumulateIntoHalfword3EStepMotionPairThenOnCountdown
 * CopyRecordWordsAddC00ToHalfword3ECallAF4A0With20000AndAdvanceWorkByte2
 * BmagicMagic01500_801F0570 passes the same cell as the first argument of the
 * same helper. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Battle byte in main RAM, outside this overlay's image, forwarded to the
 * helper at 0x800A9820 in this body's byte-0xB-zero arm. Splat's generated
 * undefined-syms map already binds the address, so the name needs no
 * target-map row. The sibling lift forwardBattleByteAndAdvanceWorkByte2When
 * HelperReadyBmagicMagic02103_801EF208 forwards the same cell to the same
 * helper. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Battle size byte in main RAM, outside this overlay's image, tested against
 * the unsigned bound 3 before the 0x800AFC98 call. Splat's generated
 * undefined-syms map already binds the address, so the name needs no
 * target-map row. The sibling lift integrateWorkWord14GrowByteATo8ThenOnByte9
 * ZeroSeedMotionSetHeadingToward801EB4E0ForwardBattleByteAndAdvanceWorkByte2
 * BmagicMagic00100_801EFD28 calls the same helper with the same (2, 0)
 * arguments. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Scratchpad work-object pointer cell. The D_ name auto-binds the address
 * through the composed Splat maps, so no target-map row or WEAK_SYMBOL_AT
 * binding is needed. A store through the loaded pointer may alias the cell, so
 * the body re-reads it before each access group: once for the byte and halfword
 * step, once for the byte 0x48 and word seed group, once for the byte 0xB test
 * and once for the byte 2 advance. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. This
 * body passes the record read through 0x8014638C, a 0x2000 value and a zero
 * third argument, the argument roles the sibling lifts of this target pass to
 * the same helper. */
extern void func_800AF4A0(u8 *record, s32 value, s32 extra); /* @source 0x800AF4A0 */

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. Its one
 * argument is forwarded from the byte at 0x80146384. */
extern void func_800A9820(u8 index); /* @source 0x800A9820 @kind function */

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. This
 * body passes 2 and 0, the constants the sibling lifts of this target pass. */
extern void func_800AFC98(s32 arg0, s32 arg1); /* @source 0x800AFC98 @kind function */

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. This
 * body passes the selector 0x203. */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C @kind function */

/* @source 0x801F0770
 * @behavior Entry 28 of the overlay handler table at 0x801EEC04, the word
 *           stored at 0x801EEC74 between the entries at 0x801EEC70 and
 *           0x801EEC78: it first passes the shared battle record read through
 *           the pointer cell 0x8014638C together with 0x2000 and a zero third
 *           argument to helper 0x800AF4A0, then steps the scratchpad work
 *           object published at the pointer cell 0x1F800044. The step reads the
 *           object once, increments countdown byte 9 and keeps the incremented
 *           value in a byte local so the truncation, not a reload, supplies the
 *           compared value, subtracts 0xC0 from work halfword 0x3E, and returns
 *           while the new countdown is not 0x14. When the countdown reaches
 *           0x14 it re-reads the cell, clears work byte 0x48, seeds work word
 *           0x14 with 0x48 and work word 0x20 with -8, and sets both words of
 *           the 16.16 motion pair at 0x40 and 0x44 to 0x10000. It then reads
 *           work byte 0xB through a further cell read and, when that byte is
 *           zero, forwards the battle byte at 0x80146384 to helper 0x800A9820,
 *           invokes 0x800AFC98 with (2, 0) while the unsigned battle byte at
 *           0x80146374 stays below 3, and invokes 0x8015D71C with the selector
 *           0x203. Both arms then re-read the cell and advance work byte 2, the
 *           handler index, with a byte-width read-modify-write. The cell is
 *           re-read before each access group because a store through the loaded
 *           pointer may alias it, and the 0x18-byte frame only keeps the return
 *           address at 0x10 across the four calls. The byte 0xB test takes its
 *           own cell read after the byte 0x48 store and holds that pointer
 *           across the four word stores, which is the representation that
 *           measured exact here: spelling the test as a fresh cell expression at
 *           the guard instead measured 54 of 61 instructions over 248 bytes,
 *           because the cell read then lands directly before its byte load and
 *           spends a fill nop on the load delay. The 0xF4-byte span ends
 *           exactly at the next subsegment, 0x801F0864, so the row claims no
 *           trailing bytes. No jal or j word in the payload targets 0x801F0770
 *           and the only word naming the address is the handler-table entry at
 *           0x801EEC74, so the caller that selects it lies outside the overlay.
 *           The 0xF4 bytes are not byte identical to any span of the 396
 *           payloads under out/binaries, which one payload scan compared. The
 *           body takes no arguments, returns nothing, and reads or writes no
 *           other state; the roles of work byte 9, work halfword 0x3E, work
 *           word 0x14, work word 0x20, work byte 0x48, the motion words 0x40 and
 *           0x44, work byte 0xB and the helper selector 0x203 are not
 *           established, so they stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callAF4A0With2000IncrementWorkByte9SubtractC0FromHalfword3EThenOnCount20ClearByte48SeedWorkWordsAndMotionPairWhenWorkByteBZeroForwardBattleByteInvokeAFC98AndD71CWith203AdvanceWorkByte2BmagicMagic01500_801F0770(void) {
  u8 *work;
  u8 countdown;
  u8 *object;
  u8 *flag;
  u8 *step;

  func_800AF4A0(D_8014638C, 0x2000, 0);

  work = D_1F800044;
  countdown = work[9] + 1;
  work[9] = countdown;
  *(u16 *)(work + 0x3E) -= 0xC0;
  if (countdown == 0x14) {
    object = D_1F800044;
    object[0x48] = 0;
    flag = D_1F800044;
    *(u32 *)(object + 0x14) = 0x48;
    *(s32 *)(object + 0x20) = -8;
    *(u32 *)(object + 0x40) = 0x10000u;
    *(u32 *)(object + 0x44) = 0x10000u;
    if (flag[0xB] == 0) {
      func_800A9820(D_80146384);
      if (D_80146374 < 3) {
        func_800AFC98(2, 0);
      }
      func_8015D71C(0x203);
    }
    step = D_1F800044;
    step[2]++;
  }
}
