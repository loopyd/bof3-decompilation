#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801BEE5C
 * @behavior walks the 0x140-byte-stride local work records at D_80145E90:
 * while the D_80146254 record count is nonzero it first scans the records'
 * leading bytes for the first cleared one, and when that index is still inside
 * the count it publishes each record at D_80146250 and as the scratchpad work
 * pointer, then selects one of four per-record paths from the flags word of the
 * 164-byte game input record in D_80144974 indexed by the record byte at 0x13C
 * and from the record route byte at 0x08 measured against D_801CD478[D_801462EC]:
 * flags bit 0x4000 requests the func_8014D664 upload from record byte 0x4B and
 * clears the record's 0x0B byte; a matching route byte with a nonzero 0x0B byte
 * uploads from the record's 0x08 byte and clears that byte; a differing route
 * byte advances the 0x08 byte by the record's 0x0B step modulo 8 before the
 * upload of the reloaded work record's 0x08 byte; and the func_8014D978 ready
 * helper then runs. With the walk finished (or the count non-positive) and bit 2
 * of D_80146325 clear, it walks the same records once more, copying each
 * 32-entry halfword row selected by the record's 0x05 byte from the 0x80035600
 * table into the row 0xC0 bytes higher, calls func_801C2438, clears the work
 * record's 0x0B byte and stores tint bytes -0x40 into 0x5D-0x5F with 1 at 0x5C.
 * Returns 0 from the first walk and 1 from the second.
 * @status partial
 * @match 76.14
 * @residual Instruction-shape residual after the clean-C ladder reached
 * expression/register order: the record walk's entry test stays peeled
 * (the loop keeps a duplicated leading flag test and a compensating index
 * decrement) and the 32-entry row copy keeps strength-reduced induction
 * variables ($a1 = 0xC0 + 2i) with the reversed addu operands the sibling
 * func_801C2438 already records; the head `lui` also allocates $v1 where the
 * original uses $v0. Live: 118/181 instructions, 724 vs 704 bytes (size delta
 * +20), first mismatch +0x0000. No pins, clobbers or barriers.
 */
s32 func_801BEE5C(void) {
  s32 count;
  s32 i;
  s32 search_offset;
  s32 j;
  s32 offset;
  u8* rec;
  u8* entries;
  u8* row;
  u8* pad;
  u16 value;

  count = D_80146254;
  i = 0;
  search_offset = 0;
  for (i = 0; i < count; i++) {
    if (D_80145E90[search_offset + 0x0B] != 0) {
      break;
    }
    search_offset += 0x140;
  }
  if (i < count) {
    i = 0;
    offset = 0;
    for (; i < D_80146254; i++, offset += 0x140) {
      rec = D_80145E90 + offset;
      D_80146250 = rec;
      g_game_work = (struct GameWorkArea*)rec;
      if ((D_80144974[rec[0x13C]].flags & 0x4000) != 0) {
        func_8014D664(rec[0x4B], 0, 0x800F0800, 0xA00);
        g_game_work->field_0B = 0;
      } else if ((rec[0x08] & 0xFF) == D_801CD478[D_801462EC]) {
        if (rec[0x0B] != 0) {
          func_8014D664(rec[0x08], 0, 0x800F0800, 0xA00);
          g_game_work->field_0B = 0;
        }
        func_8014D978();
      } else {
        rec[0x08] = (rec[0x08] + rec[0x0B]) & 7;
        func_8014D664(g_game_work->route_index_08, 0, 0x800F0800, 0xA00);
        func_8014D978();
      }
    }
    return 0;
  }

  if ((D_80146325 & 4) != 0) {
    return 1;
  }
  count = D_80146254;
  if (count > 0) {
    entries = PSX_PTR(u8, 0x80035600u);
    i = 0;
    offset = 0;
    for (; i < D_80146254; i++, offset += 0x140) {
      rec = D_80145E90 + offset;
      D_80146250 = rec;
      g_game_work = (struct GameWorkArea*)rec;
      for (j = 0; j < 0x20; j++) {
        row = entries + (rec[0x05] << 6);
        pad = row + 0xC0;
        value = *(u16*)(row + (j << 1));
        *(u16*)(pad + (j << 1)) = value;
      }
      func_801C2438();
      g_game_work->field_0B = 0;
      g_game_work->unk_5D = -0x40;
      g_game_work->unk_5E = -0x40;
      g_game_work->unk_5F = -0x40;
      g_game_work->flags_5C = 1;
    }
  }
  return 1;
}
