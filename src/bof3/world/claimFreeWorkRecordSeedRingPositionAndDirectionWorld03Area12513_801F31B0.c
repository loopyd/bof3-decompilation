#include "bof3/bof3.h"

u8* func_801F3348(void);

/* The overlay's per-slot spawn tables, 16 entries of two signed halfwords each:
 * the ring positions at 0x801F364C and the direction vectors they are the fixed
 * center 0x1980,0x1680 plus, 0x40 bytes later at 0x801F368C. Both live in the
 * single data blob claimed by the textbin row at D_801F364C. */
extern s16 D_801F364C[16][2];
extern s16 D_801F368C[16][2];

/* @source 0x801F31B0
 * @behavior Work-record spawn from this overlay's per-slot tables: it asks the
 *           overlay scan at 0x801F3348 for the first free entry of the
 *           0x10-entry, 0x28-byte-stride record table at 0x800E4800 and returns
 *           without writing anything when every entry is claimed. Otherwise it
 *           claims the record with 1 at byte 0, clears bytes 1 and 2, stores
 *           0x20 at byte 3 and the halfword 0x80 at offset 4, then fills four
 *           words from the tables the argument selects: the word at 0x08 takes
 *           the first signed halfword of the entry at 0x801F364C that the
 *           unsigned argument indexes at the four-byte table stride, shifted
 *           left 8, the word at 0x0C takes that entry's second halfword the same
 *           way, the word at 0x18 takes the first halfword of the same index in
 *           the table at 0x801F368C shifted left 5 and the word at 0x1C its
 *           second halfword, while the words at 0x10 and 0x20 are cleared. Both
 *           tables hold 16 entries; entry i of the first is the fixed center
 *           0x1980,0x1680 plus entry i of the second, so the seeded words at
 *           0x08 and 0x0C are a ring point and the words at 0x18 and 0x1C the
 *           matching direction vector of the 16-step ring. The original reads
 *           each halfword through its own materialised table address, so every
 *           load spells the table base again rather than sharing one. It takes
 *           one argument, returns nothing, and its 0x18-byte frame keeps $ra
 *           plus $s0 for the slot, which survives the scan call; the epilogue
 *           restores the frame before jr, leaving that delay slot unfilled.
 *
 * Name evidence: the only in-image callers are this overlay's own spawners.
 *           func_801F2C80 calls it at 0x801F2C88, 0x801F2C90 and 0x801F2C98 with
 *           6, 9 and 0xD and then arms the work object byte 9 with 0x3C and
 *           advances work byte 1, and func_801F3268 fills a stack array with the
 *           bytes 0 to 15, shuffles it with rand, and calls this address once
 *           per entry of the shuffled list up to its own argument, so the
 *           argument is a ring slot index and the caller above covers the ring
 *           in shuffled order. The callee is this target's free-entry scan and
 *           the shape is this overlay family's claim-then-seed record spawn, as
 *           in claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0 and
 *           claimFreeWorkRecordSeedWorkPositionAndAngleVectorScenarioScena0800_801F802C,
 *           whose record tables are the same 0x800E4800 table and whose claim
 *           byte, cleared bytes and 0x80 halfword at offset 4 are the same
 *           fields; the ring position and direction words are this overlay's
 *           own table contents, so the roles stay described by offset.
 *
 * Shape evidence: the claim, the early return on a zero scan result and the
 *           claim byte / cleared bytes / halfword 0x80 statement order follow
 *           the two exact siblings above; every literal, table address, shift,
 *           index mask and offset here comes from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeWorkRecordSeedRingPositionAndDirectionWorld03Area12513_801F31B0(s32 slot) {
  u8* record;
  s32 index;

  record = func_801F3348();
  if (record == 0) {
    return;
  }
  index = slot & 0xFF;
  record[0] = 1;
  record[1] = 0;
  record[2] = 0;
  record[3] = 0x20;
  *(u16*)(record + 4) = 0x80;
  *(s32*)(record + 0x08) = D_801F364C[index][0] << 8;
  *(s32*)(record + 0x0C) = D_801F364C[index][1] << 8;
  *(u32*)(record + 0x10) = 0;
  *(s32*)(record + 0x18) = D_801F368C[index][0] << 5;
  *(s32*)(record + 0x1C) = D_801F368C[index][1] << 5;
  *(u32*)(record + 0x20) = 0;
}
