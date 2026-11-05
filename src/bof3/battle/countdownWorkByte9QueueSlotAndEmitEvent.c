#include "bof3/battle/battle03_internal.h"

void func_8014D6B8(u8 request); /* @source 0x8014D6B8 */

/* @source 0x801E0F0C
 * @behavior Counts down the scratch work byte +0x09 and returns while it is
 * still nonzero. On expiry it re-arms the published local work record
 * D_80146250 -- flag 0x4000 of halfword +0x80 is cleared, bit 0x4 of word
 * +0x124 is cleared, halfword +0x88 becomes 1 and byte +0x8C is halved -- then
 * allocates a queued slot store through func_801E590C(0, 1) and marks that
 * entry with byte +0x07 = 1, byte +0x27 = 6 and the scratch work pointer in
 * +0x74. It requests resource scratch byte +0x08 plus 4 through func_8014D6B8
 * and func_8015D71C(0x206), and while the dispatched-handler gate byte
 * D_80146329 is clear it initializes the UI bundle slot, substitutes the name
 * of the local work record selected by scratch byte +0x05 and queues the
 * event row addressed by work byte +0x79. Both paths finish with scratch byte
 * +0x03 set to 2 and localReadyOrHelper1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownWorkByte9QueueSlotAndEmitEvent(void) {
  Battle03LocalWork* scratch;
  Battle03LocalWork* work;
  Battle03LocalWork* queued_work;
  u32 slot;

  scratch = D_1F800044;
  if (--scratch->unk_09 != 0u) {
    return;
  }

  work = D_80146250;
  work->unk_80 &= ~0x4000u;
  work->unk_124 &= ~4u;
  work->unk_88 = 1;
  work->unk_8c >>= 1;

  slot = (u8)func_801E590C(0u, 1u);
  queued_work = D_1F800044;
  D_801EC330[slot].pad_07[0] = 1;
  FIELD_REF(u8, &D_801EC330[slot], 0x27) = 6;
  D_801EC330[slot].ptr_74 = (u32)queued_work;

  func_8014D6B8(D_1F800044->unk_08 + 4u);
  func_8015D71C(0x206u);

  if (D_80146329 == 0u) {
    initUiBundleSlot0();
    func_801DE9A8(D_1F800044->unk_05);
    queueScriptEvent(0u, D_80146250->unk_79);
  }

  D_1F800044->unk_03 = 2;
  localReadyOrHelper1();
}
