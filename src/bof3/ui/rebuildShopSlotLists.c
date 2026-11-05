#include "bof3/ui/shop00_internal.h"

/* @source 0x801DE200
 * @behavior shop sub-step handler reached as entry 0 of the sub-step table
 *           D_801E5BFC, which the phase dispatcher func_801DE1C4 indexes with
 *           the UI sub-step byte D_80148652: it reads the visible-list length
 *           through func_801BDB7C(0), clears the off-screen count byte
 *           D_801E6250 and appends the id byte (record offset +5) of every one
 *           of the eight 164-byte field records of D_80144968 whose low flag bit
 *           D_8014496F is set and whose id the visible list D_80144F5A does not
 *           already hold. It then sets the first byte of the three slot pairs
 *           D_801E6248 to 1 for the visible slots and clears the unused ones to
 *           0, sets the sibling byte D_801E6249 of every slot the off-screen
 *           list fills to 1, re-arms the saved selection cursor pair
 *           D_8014865A/D_8014865D with the 0x7F no-slot sentinel and the current
 *           pair D_80148658/D_80148659 with kind 0 slot 0, re-arms the pair byte
 *           D_801E6254 with 0xFF, arms the frame timer phaseTimer with 3,
 *           advances the UI sub-step byte and switches the frontend-local mode
 *           through func_8014ECAC with the bare constant 2.
 * @status partial
 * @match 89.57
 * @residual 103/115 insns match, 460 bytes versus the original 456; the first
 *           difference at +0x18 is the record flag address: the original uses
 *           a folded symbol-plus-offset load, while this form reduces it to a
 *           pointer. The first and third pair loops also hoist their offset
 *           reset; the third increments its slot before the store, leaving an
 *           extra load-delay nop. The guarded middle clear loop matches.
 */
/* After the visible slots are marked, i is dead as a slot index. The guarded
 * clear loop reuses it as a byte offset: 2 * i addresses the first remaining
 * pair, and offsets below 6 cover exactly the remaining pairs below slot 3.
 * The guard is required because the visible count is an unrestricted u8.
 */
void rebuildShopSlotLists(void) {
  u8  count;
  s32 slot;
  s32 i;
  s32 offset;

  count = func_801BDB7C(0);
  D_801E6250 = 0;
  for (i = 0; i < 8; i++) {
    if ((D_8014496F[i * 0xA4] & 1) != 0) {
      for (slot = 0; slot < count; slot++) {
        if (D_80144968[i * 0xA4 + 5] == D_80144F5A[slot]) {
          break;
        }
      }
      if (slot == count) {
        D_801E6244[D_801E6250++] = D_80144968[i * 0xA4 + 5];
      }
    }
  }
  i = 0;
  offset = 0;
  for (; i < count; i++, offset += 2) {
    D_801E6248[offset] = 1;
  }
  if (i < 3) {
    i *= 2;
    do {
      D_801E6248[i] = 0;
      i += 2;
    } while (i < 6);
  }
  i = 0;
  offset = 0;
  for (; i < D_801E6250; i++, offset += 2) {
    D_801E6249[offset] = 1;
  }
  D_8014865A = 0x7F;
  D_8014865D = 0x7F;
  D_801E6254 = 0xFF;
  D_80148658 = 0;
  D_80148659 = 0;
  phaseTimer = 3;
  D_80148652++;
  func_8014ECAC(2);
}
