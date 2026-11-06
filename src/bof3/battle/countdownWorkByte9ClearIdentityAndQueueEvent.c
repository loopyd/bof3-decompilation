#include "bof3/battle/battle03_internal.h"

void func_8014D6B8(u8 request); /* @source 0x8014D6B8 */

/* @source 0x801E10C8
 * @behavior Counts down the scratch work byte +0x09 and returns while it is
 * still nonzero. On expiry it clears the identity byte matching the current
 * work-state byte -- +0x86 while that byte reads 3, +0x85 followed by
 * func_801E0B64 while it reads 5, +0x87 while it reads 6 -- and re-arms the
 * published local work record D_80146250: flag 0x4000 of halfword +0x80 is
 * cleared and halfword +0x88 and byte +0x8C take the values of halfword +0x90
 * and byte +0x9E. It then allocates a queued slot store through
 * func_801E590C(0, 1) and marks that entry with byte +0x07 = 2, byte +0x27 = 6
 * and the scratch work pointer in +0x74, requests resource scratch byte +0x08
 * plus 4 through func_8014D6B8 and func_8015D71C(0x206), and while the
 * dispatched-handler gate byte D_80146329 is clear it initializes the UI bundle
 * slot and queues event row kind 2 with halfword 0x2D and the texture
 * func_801502D0(0x2E) -- func_801502D0(0x3B) while the work-state byte reads 5.
 * Both paths finish with scratch byte +0x03 set to 2 and localReadyOrHelper1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownWorkByte9ClearIdentityAndQueueEvent(void) {
  Battle03LocalWork* scratch;
  Battle03LocalWork* work;
  Battle03LocalWork* queued_work;
  u32 slot;
  u32 texture;

  scratch = D_1F800044;
  if (--scratch->unk_09 != 0u) {
    return;
  }

  if (D_1F800044->unk_03 == 3u) {
    D_80146250->unk_86 = 0;
  }
  if (D_1F800044->unk_03 == 5u) {
    D_80146250->unk_85 = 0;
    func_801E0B64();
  }
  if (D_1F800044->unk_03 == 6u) {
    D_80146250->unk_87 = 0;
  }

  work = D_80146250;
  work->unk_80 &= ~0x4000u;
  work->unk_88 = work->unk_90;
  work->unk_8c = work->unk_9e;

  slot = (u8)func_801E590C(0u, 1u);
  queued_work = D_1F800044;
  D_801EC330[slot].pad_07[0] = 2;
  FIELD_REF(u8, &D_801EC330[slot], 0x27) = 6;
  D_801EC330[slot].ptr_74 = (u32)queued_work;

  func_8014D6B8(D_1F800044->unk_08 + 4u);
  func_8015D71C(0x206u);

  if (D_80146329 == 0u) {
    initUiBundleSlot0();
    texture = func_801502D0(D_1F800044->unk_03 == 5u ? 0x2Eu : 0x3Bu);
    func_801DE560(2u, 0u, 0u, 0x2Du, texture);
  }

  D_1F800044->unk_03 = 2u;
  localReadyOrHelper1();
}
