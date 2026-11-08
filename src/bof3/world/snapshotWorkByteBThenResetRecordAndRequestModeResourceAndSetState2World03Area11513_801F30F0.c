#include "bof3/bof3.h"

extern u8* D_1F800044;
extern u16 D_80143F10;
extern u16 D_801F4D7C[];
extern u8 D_801F4D7E[];

/* Shared resident resource request at 0x8014D6B8: takes the resource id and
 * consumes no result. */
void func_8014D6B8(u32 resource);

/* @source 0x801F30F0
 * @behavior Snapshots the work-record byte 0x0B into byte 0x07, then acts on
 *           the entry mode it read there, always through a fresh read of the
 *           work record published at the scratchpad pointer cell 0x1F800044.
 *           Mode 1 walks the halfword keys of the local table based at
 *           0x801F4D7C in four-byte steps until one equals the shared seed
 *           0x80143F10, stores that key as a word at record offset 0x18 and
 *           takes the resource id from the companion byte at 0x801F4D7E for
 *           the same entry. Modes 2, 3 and 4 take resource ids 3, 0 and 1, and
 *           any other mode returns without a request. Modes 1 to 4 all clear
 *           the record word at 0x40, store 0x10000 as a word at 0x44, store 2
 *           to byte 0x48 and 8 to byte 0x09, request that resource through the
 *           shared helper 0x8014D6B8, and store the state 2 to record byte
 *           0x01. Takes no arguments, uses no result and saves only the return
 *           address and one callee-saved register. The 0x138 bytes are the
 *           same body as the not yet lifted func_801F30EC of
 *           emi/world00/area016/13 except for the five internal jump
 *           displacements, which encode that overlay own addresses; this
 *           target carries the identical table and seed cell addresses.
 * @status partial
 * @match 69.23
 * @residual 54/78 instructions match; the body is 296 bytes versus 312.
 *           The first difference is the branch target at +0x34. The original
 *           reloads the work byte for modes 2 to 4, while this body reuses the
 *           mode-1 test value. Seed and table offset occupy the opposite
 *           argument registers, the offset copy is absent, and state/record
 *           initialization schedules differently. Retained for review.
 */
void snapshotWorkByteBThenResetRecordAndRequestModeResourceAndSetState2World03Area11513_801F30F0(
    void) {
  u8* record;
  u32 seed;
  u32 offset;
  u32 resource;
  u8 state;

  record = D_1F800044;
  record[0x07] = record[0x0B];
  if (D_1F800044[0x0B] == 1) {
    seed = D_80143F10;
    offset = 0;
  search:
    if (seed == FIELD_REF(u16, D_801F4D7C, offset)) {
      goto found;
    }
    offset += 4;
    goto search;
  }
  if (D_1F800044[0x0B] == 2) {
    resource = 3;
    goto reset;
  }
  if (D_1F800044[0x0B] == 3) {
    resource = 0;
    goto reset;
  }
  goto mode4;
found:
  record = D_1F800044;
  state = 2;
  FIELD_REF(u32, record, 0x18) = seed;
  FIELD_REF(u32, record, 0x40) = 0;
  FIELD_REF(u32, record, 0x44) = 0x10000;
  record[0x48] = state;
  D_1F800044[0x09] = 8;
  resource = FIELD_REF(u8, D_801F4D7E, offset);
  goto request;
mode4:
  if (D_1F800044[0x0B] != 4) {
    return;
  }
  resource = 1;
reset:
  record = D_1F800044;
  state = 2;
  FIELD_REF(u32, record, 0x40) = 0;
  FIELD_REF(u32, record, 0x44) = 0x10000;
  record[0x48] = state;
  D_1F800044[0x09] = 8;
request:
  func_8014D6B8(resource);
  D_1F800044[0x01] = state;
}
