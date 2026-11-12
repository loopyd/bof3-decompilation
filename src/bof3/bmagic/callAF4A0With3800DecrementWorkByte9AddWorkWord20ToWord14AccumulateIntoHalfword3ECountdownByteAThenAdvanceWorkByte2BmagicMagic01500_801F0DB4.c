#include "bof3/bof3.h"

/* Shared battle record pointer in main RAM, outside this overlay's image.
 * Splat's generated undefined-syms map already binds the address, so the name
 * needs no target-map row. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell. This target's map binds no name at the
 * address, so the D_ spelling auto-binds with no map row, as it does for the
 * scratchpad lifts of this target that already measure exact. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. */
extern void func_800AF4A0(u8 *record, s32 value, s32 extra); /* @source 0x800AF4A0 */

/* @source 0x801F0DB4
 * @behavior Entry 33 of the overlay handler code-pointer run that starts at
 *           0x801EEC04, the word the tabledata segment stores at 0x801EEC88: it
 *           first passes the shared battle record read through the pointer cell
 *           0x8014638C together with 0x3800 and a zero third argument to helper
 *           0x800AF4A0, then steps the scratchpad work object published at the
 *           pointer cell 0x1F800044. The step decrements work byte 9, adds work
 *           word 0x20 to work word 0x14, accumulates the updated word 0x14 into
 *           work halfword 0x3E, then decrements work byte 0xA and, when that
 *           byte reaches zero, advances work byte 2, the handler index. The
 *           original re-reads the pointer cell before the byte 0xA group and
 *           again before the byte 2 advance because a store through the loaded
 *           pointer may alias the cell; the 0x18-byte frame only keeps $ra
 *           across the helper call. No jal word in the payload targets
 *           0x801F0DB4, so the caller that selects this table entry lies outside
 *           the overlay. Takes no arguments, returns nothing and reads or writes
 *           no other state; work-field roles remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callAF4A0With3800DecrementWorkByte9AddWorkWord20ToWord14AccumulateIntoHalfword3ECountdownByteAThenAdvanceWorkByte2BmagicMagic01500_801F0DB4(void) {
  u8 *work;
  u8 *countdown;
  s32 step;
  u8 count;

  func_800AF4A0(D_8014638C, 0x3800, 0);
  work = D_1F800044;
  step = *(s32 *)(work + 0x14) + *(s32 *)(work + 0x20);
  work[9]--;
  countdown = D_1F800044;
  *(u16 *)(work + 0x3E) += step;
  *(s32 *)(work + 0x14) = step;
  count = (u8)(countdown[0xA] - 1);
  countdown[0xA] = count;
  if (count == 0) {
    D_1F800044[2]++;
  }
}
