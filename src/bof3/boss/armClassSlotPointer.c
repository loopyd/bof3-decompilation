#include "bof3/bof3.h"

/* Current enemy work record pointer cell in main RAM, outside this overlay's
 * image: the main program publishes the record the overlays run against and the
 * exact same-target lift at 0x800C1858 reads the same cell. Every access here
 * is byte-addressed, so the declaration keeps the byte view only. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratch work record pointer cell in the scratchpad at 0x1F800044: the main
 * program publishes the record there, this target's map already binds that
 * word as D_1F800044, and both overlay dispatchers index the record the cell
 * holds. The original materializes `lui`/`lw` of 0x1F800044 once per read, so
 * the declaration is a pointer variable and not an address-macro lvalue. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Action-slot pool word of the battle image at 0x801EC3A4 in main RAM, outside
 * this overlay: entries are 0x78 bytes apart, each field is reached through its
 * own flat symbol -- `lui $at,%hi(SYM)`, `addu $at,$at,index*0x78`, store
 * `%lo(SYM)($at)` -- and this body writes the entry's published work-record
 * pointer at that word. The exact boss lift at 0x800C1950 of
 * BIN/BOSS/BOSS028.EMI#16 publishes the same word of the entry it claims. */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable action-slot allocator at 0x801E590C, bound through
 * this target's generated undefined_funcs_auto.txt. It stores its first
 * argument at entry byte +0x06 and its second at +0x05 and returns the claimed
 * entry index, or 0xFF when the pool is full; this body truncates the returned
 * word to a byte before scaling it and never tests that failure code, so the
 * declaration keeps the raw argument order. The exact battle03 lift of this
 * address measures the same contract. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x800C1900
 * @behavior Entry 0 of this overlay's byte-2 step run, the code pointer run the
 *           payload places at 0x800C1DF4 and the dispatcher
 *           dispatchStateByte2Table1df4BossBoss05116_800C18BC indexes with work
 *           byte +2 of the scratch work record at 0x1F800044. It loads the
 *           current enemy work record pointer from the main-RAM cell
 *           0x801EB4E8, takes that record's unsigned byte +0xE0, scales it by
 *           the 0x88-byte class-row stride and stores the class-row byte at
 *           0x800E40CE into scratch work byte +0x09. It then claims an action
 *           slot of the 0x78-byte pool through func_801E590C(3, 7), truncates
 *           the returned index to a byte, scales it by the stride and publishes
 *           the work record pointer it re-reads from the scratchpad cell into
 *           the claimed slot word at 0x801EC3A4, and finally advances scratch
 *           work byte +0x02 by one. That step advance moves the dispatcher to
 *           run word 1, the inert handler 0x800C1984, so the body runs once per
 *           step; the 0xFF "pool full" answer is not distinguished and only
 *           scales the published word out of the pool. The body has no branch:
 *           the record pointer load precedes frame creation, the class-row
 *           store fills the call's delay slot, and the 0x18-byte frame only
 *           keeps $ra at 0x10 across the single call. No jal names 0x800C1900;
 *           its only in-image reference is the run word at 0x800C1DF4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armClassSlotPointer(
    void) {
  u8 *work;
  u32 classIndex;
  u8 *state;
  u32 slot;
  u8 *published;

  work = D_801EB4E8;
  classIndex = work[0xE0];
  state = D_1F800044;
  state[9] = PSX_REF(u8, 0x800E40CEu + classIndex * 0x88u);
  slot = (u8)func_801E590C(3, 7) * 0x78;
  published = D_1F800044;
  *(u8 **)&D_801EC3A4[slot] = published;
  published[2]++;
}
