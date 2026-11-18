#include "bof3/bof3.h"

extern u8 *boss028CurrentWorkRecordState; /* @source 0x1F800044 @kind data */

/* Current enemy work record pointer cell in main RAM, outside this overlay's
 * image. Its word 0xE8 is the in-row pointer whose halfword 0 carries the
 * effect id this body submits; the exact sibling at 0x800C18E4 of this target
 * reads the same cell and the exact boss lift at 0x800C1F0C of
 * BIN/BOSS/BOSS025.EMI#13 submits the same word-0xE8 halfword, so the
 * declaration keeps the byte view only and the record's other fields stay
 * offset-anchored. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Action-slot pool of the battle image at 0x801EC330 in main RAM, outside this
 * overlay: 0x78-byte stride, with each field reached through its own flat
 * symbol - `lui $at,%hi(SYM)`, `addu $at,$at,index*0x78`, store `%lo(SYM)($at)`
 * - and the scaled index in one register shared by both stores. Slot byte
 * +0x01 keeps its address anchor here: this body clears it to 0, where the
 * exact same-target sibling at 0x800C1C9C stamps 1. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-executable readiness helper at 0x801E30F8, outside this overlay's
 * image and bound through this target's generated undefined-funcs map. The
 * exact same-target lift at 0x800C1AA0 declares the same unsigned byte return
 * and tests it for zero, and the exact boss lift at 0x800C1F0C of
 * BIN/BOSS/BOSS025.EMI#13 records the same helper. */
extern u8 func_801E30F8(void); /* @source 0x801E30F8 @kind function */

/* Shared main-executable effect submitter at 0x801E52F0, bound through this
 * target's generated undefined-funcs map. The exact lifts at 0x800C1F0C of
 * BIN/BOSS/BOSS025.EMI#13 and at 0x800C1A58 of BIN/BOSS/BOSS033.EMI#16 call it
 * with the halfword effect id they read from the current enemy work record and
 * declare the same one-argument prototype. */
extern void func_801E52F0(u16 effect_id); /* @source 0x801E52F0 */

/* Slot allocator of the battle image at 0x801E590C, outside this overlay and
 * bound through this target's generated undefined_funcs_auto.txt. It returns
 * the index of the claimed action slot, which this body truncates to a byte
 * before scaling it by the 0x78-byte stride, exactly as the sibling at
 * 0x800C1C9C of this target does for its own claim. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x800C1950
 * @behavior Entry 1 of this overlay's byte-2 step run, the code pointer run
 *           based at 0x800C2184 that the dispatcher
 *           dispatchWorkByte2Table2184BossBoss02816_800C18A0 indexes with work
 *           byte +2 of the scratch work record at 0x1F800044; this address is
 *           the run's word 1 at 0x800C2188 and no jal in the payload encodes
 *           it. Run word 0 at 0x800C18E4 arms work byte +0x09 from the
 *           class-row byte and advances that step index, which is how the
 *           dispatcher reaches this body. It loads the work record pointer from
 *           the scratchpad cell before the frame reservation, decrements that
 *           record's unsigned byte +0x09 in place and tests the stored byte
 *           truncated to eight bits, so the guarded arm runs only when the
 *           countdown reaches zero. On zero it reads the current enemy work
 *           record pointer at 0x801EB4E8, takes that record's word 0xE8 and
 *           submits the halfword at its offset 0 through the shared submitter
 *           0x801E52F0.
 *           It then calls the shared readiness helper 0x801E30F8 and takes the
 *           one epilogue while the low byte of the answer is zero. When the
 *           helper reports, it claims an action slot of the 0x78-byte pool:
 *           it truncates the index func_801E590C(3, 2) returns to a byte,
 *           scales it by the stride into one register shared by both stores,
 *           clears the slot byte at 0x801EC331 to 0, publishes the work record
 *           pointer it re-reads from the scratchpad cell in the slot word at
 *           0x801EC3A4, and finally advances work byte +0x02 by one at byte
 *           width, handing the byte-2 dispatcher to entry 2 at 0x800C1A08,
 *           this overlay's noop handler.
 *           The 0x18-byte frame only keeps $ra at 0x10 across the three calls,
 *           the body takes no arguments, every delay slot in the span is a nop,
 *           and the value left in $v0 is the incremented step byte, which the
 *           indirect step call discards. The roles of work bytes 0x09 and 0x02
 *           as the armed countdown and the step index follow the exact
 *           same-target runners at 0x800C18E4 and 0x800C1C9C; the slot byte and
 *           word offsets stay address-anchored.
 *           Shape evidence: no payload under out/binaries holds a
 *           byte-identical copy of this 0xB8-byte span (a scan of all 398
 *           extracted payloads hit only this target at payload offset 0x150), so
 *           it has no clone seed. Four exact siblings supply the statement
 *           order and operand forms: BIN/BOSS/BOSS025.EMI#13 0x800C1F0C for the
 *           in-place byte-9 countdown, the zero test and the guarded
 *           0x801E52F0 submit of the word-0xE8 halfword; this target's
 *           0x800C1AA0 for the 0x801E30F8 byte-return zero test; this target's
 *           0x800C1C9C for the slot claim, the 0x78 stride into one register
 *           shared by both stores, the 0x801EC3A4 publication and the byte-2
 *           advance; and this target's 0x800C18E4 for the cell re-read and the
 *           byte-width step advance. The stored slot byte and the added
 *           readiness guard are this body's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countDownWorkByte9SubmitWorkEffectOnExpiryThenOnEnemyReadyClaimActionSlotClearSlotBytePublishWorkPointerAndAdvanceWorkByte2BossBoss02816_800C1950(
    void) {
  u8 *work;
  u8 countdown;
  u8 *published;
  u32 slot;

  work = boss028CurrentWorkRecordState;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    func_801E52F0(FIELD_REF(u16, FIELD_REF(u8 *, D_801EB4E8, 0xE8u), 0u));
  }
  if (func_801E30F8() != 0) {
    slot = (u8)func_801E590C(3, 2) * 0x78;
    D_801EC331[slot] = 0;
    published = boss028CurrentWorkRecordState;
    *(u8 **)&D_801EC3A4[slot] = published;
    published[2]++;
  }
}
