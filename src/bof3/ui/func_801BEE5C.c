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
 * @match 89.77
 * @residual Prologue and walk-header scheduling residual after the clean-C
 * ladder. The compiled function has the original's 176 instructions and 704
 * bytes with the original's -64 frame; both walks read D_80146254 directly, so
 * the scan bound, the post-scan test and the `blez`/`move a0,v0` count copy
 * match, and the inner row-copy load and store emit the original's
 * `sll`/`addu` operand order and register roles. Remaining classes: the frame
 * adjust and the scan-offset zeroing are scheduled ahead of the record-count
 * load instead of after it; the first walk's scan-exit branch takes the offset
 * init in its delay slot where the original emits `nop` and zeroes the index
 * first; and the second walk keeps its `if (count > 0)` pre-test, which
 * duplicates the loop's entry test, so the original's `addu v0,s1,v0` /
 * `move a2,v0` record copy has no instruction slot. Measured and rejected on
 * this allocation: the second-walk record address built as a u32 and then
 * assigned (155/176 at 708 bytes), a signed second-walk loop bound and a
 * swapped first-walk init order (158/176, neutral), dropping the second-walk
 * pre-test with its count local (144/176 at 696 bytes), a single record local
 * for both walks (148/176), and the inner row-copy load reordered before the
 * pad computation (151/176).
 */
s32 func_801BEE5C(void) {
  s32 count;
  s32 i;
  s32 search_offset;
  s32 j;
  s32 offset;
  u8* rec;
  u8* rec2;
  u32 base;
  u8* row;
  u8* pad;
  u16 value;

  i = 0;
  search_offset = 0;
  for (i = 0; i < D_80146254; i++) {
    if (D_80145E90[search_offset + 0x0B] != 0) {
      break;
    }
    search_offset += 0x140;
  }
  if (i < D_80146254) {
    i = 0;
    offset = 0;
    for (; i < D_80146254;) {
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
      offset += 0x140;
      i++;
    }
    return 0;
  }

  if ((D_80146325 & 4) != 0) {
    return 1;
  }
  count = D_80146254;
  if (count > 0) {
    base = 0x80035600u;
    i = 0;
    offset = 0;
    for (; i < D_80146254;) {
      rec2 = D_80145E90 + offset;
      D_80146250 = rec2;
      g_game_work = (struct GameWorkArea*)rec2;
      for (j = 0; j < 0x20; j++) {
        row = (u8*)(((u32)rec2[0x05] << 6) + base);
        pad = row + 0xC0;
        value = *(u16*)((u32)(j << 1) + (u32)row);
        *(u16*)((u32)(j << 1) + (u32)pad) = value;
      }
      func_801C2438();
      g_game_work->field_0B = 0;
      g_game_work->unk_5D = -0x40;
      g_game_work->unk_5E = -0x40;
      g_game_work->unk_5F = -0x40;
      g_game_work->flags_5C = 1;
      offset += 0x140;
      i++;
    }
  }
  return 1;
}
