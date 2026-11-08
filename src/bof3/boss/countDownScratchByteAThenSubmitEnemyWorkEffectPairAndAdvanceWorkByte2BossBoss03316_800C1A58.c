#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each load folds to lui plus an offset load, and because a store
 * through the loaded pointer may alias the cell the original re-reads it for the
 * step-index advance instead of keeping the pointer across the two calls. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and this target's siblings read back. Its word at offset 0xE8 is the effect
 * table this overlay publishes there from its own word 0x800C2750. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Shared main-executable effect submitter carried at 0x801E52F0 as
 * submitEffectUnlessNeg1(u16 effect_id): it forwards the id to the shared cue
 * dispatcher unless the caller passes 0xFFFF. Both ids this body submits are
 * table values, so the 0xFFFF guard never fires from here. */
extern void func_801E52F0(u16 effect_id);

/* @source 0x800C1A58
 * @behavior Byte-1 handler entry 13 of this overlay code-pointer run based at
 *           0x800C275C, the table the overlay entry dispatcher func_800C1804
 *           indexes with work byte 1 scaled by four; run word 13 at 0x800C2790
 *           holds this address and no jal in this payload names it. It loads the
 *           scratch work object published at the pointer cell 0x1F800044 before
 *           it creates its frame, decrements that object unsigned byte 0xA as a
 *           countdown, stores it back and returns while the stored byte is
 *           non-zero, so the countdown store happens on both paths. When the
 *           countdown reaches zero it reads the current enemy work record
 *           published at 0x801EB4E8 and the word at that record offset 0xE8, the
 *           effect table func_800C1848 publishes there from this overlay word
 *           0x800C2750, and submits that table halfword 0 and the same halfword
 *           plus four through the shared main-executable submitter
 *           func_801E52F0. It then re-reads the scratch cell and advances object
 *           byte 2, the work step index, by one so the next dispatch selects the
 *           following step of the handler run based at 0x800C27B4. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps the
 *           return address at 0x10 across its two calls. The roles of the
 *           countdown byte and of the submitted ids are not established here, so
 *           the name describes the stores and the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countDownScratchByteAThenSubmitEnemyWorkEffectPairAndAdvanceWorkByte2BossBoss03316_800C1A58(
    void) {
  u8 *work;
  u8 remaining;

  work = D_1F800044;
  remaining = work[0xA] - 1;
  work[0xA] = remaining;
  if (remaining == 0) {
    /* The original materialises the record word at 0xE8 into the temporary
     * register and loads the halfword from it, so both effect ids are read from
     * a freshly published table rather than from one kept across the call. */
    func_801E52F0(FIELD_REF(u16, FIELD_REF(u8 *, D_801EB4E8, 0xE8u), 0u));
    func_801E52F0(FIELD_REF(u16, FIELD_REF(u8 *, D_801EB4E8, 0xE8u), 0u) + 4);
    work = D_1F800044;
    work[2] = work[2] + 1;
  }
}
