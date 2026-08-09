#include "bof3/core/slus_internal.h"

extern u8*          D_80146844;
extern u8*          D_80146848;
extern volatile u32 D_80146464;
extern u32          D_80146678;
extern u32          D_8014667C;
extern const u8     D_80183224[];

/* @behavior validates the active EMI header tag and builds cumulative sector
 * offsets for its entries; invalid tags disable the active load.
 * @source 0x80162500
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void validateEmiLoaderHeader(void) {
  u8*           header;
  u8*           tag;
  s32           tag_count;
  s32           tag_index;
  s32           prev_count;
  u32*          entry;
  u32*          offsets;
  u32*          sector_offsets;
  s32*          entry_count;
  u32           next_offset;
  u32           header_address;
  u8*           loader_state;
  volatile u32* loader_word;

  tag_index = 7;
  loader_word = &D_80146464;
  loader_state = (u8*)loader_word + 0x30;
  header_address = *loader_word;
  tag_count = 7;
  header = (u8*)header_address;
  D_80146848 = header;
  D_80146844 = header;

  do {
    tag = header + tag_index;
    if (tag[8] != D_80183224[tag_count]) {
      loader_state[0] = 0;
      loader_state[-0x14] = 1;
      return;
    }
    /*
     * MATCHING_AID:
     * The explicit prev_count copy reproduces the original's `move $v0,$a0`
     * in the tag-compare beq delay slot and the `addiu $a0,-1; bnez $v0;
     * addiu $a1,-1` tail order (asm-diff first=+0x005c before this aid).
     * Writing `tag_index--; } while (tag_count-- != 0);` lets GCC swap the
     * two independent decrements around the bnez. Remove if the scheduler
     * behavior is reproduced without the temp.
     */
    prev_count = tag_count;
    tag_count--;
    tag_index--;
  } while (prev_count != 0);

  sector_offsets = &D_80146678;
  next_offset = sector_offsets[0] + 1;
  sector_offsets[1] = next_offset;
  entry_count = (s32*)D_80146848;
  tag_index = 2;
  if (*entry_count < 2) {
    return;
  }

  entry = (u32*)(D_80146844 + 0x20);
  offsets = sector_offsets + 2;
  do {
    next_offset += (entry[-4] + 0x7ffu) >> 11;
    *offsets = next_offset;
    entry += 4;
    tag_index++;
    offsets++;
  } while (tag_index <= *entry_count);
}
