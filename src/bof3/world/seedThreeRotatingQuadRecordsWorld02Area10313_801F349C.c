#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, read once.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x801F349C
 * @behavior Seeds the three 0x4BC-byte records of the table at 0x800E4800:
 *           copies the scratch work-record words at 0x34/0x38/0x3C into record
 *           words 0x00/0x04/0x08 (the third biased by 0x03800000), stores 0xC00
 *           in the record halfword at 0x12 and 0 in the halfword at 0x14, and
 *           gives record i the turn angle (i * 0x1000) / 3, so the three records
 *           start 120 degrees apart. Takes no arguments and returns nothing.
 *           The records are the rotating semi-transparent quad state the
 *           per-frame drawers 0x801F2CB0 / 0x801F2E5C walk with the same 0x4BC
 *           stride and 0x801F2FB8 renders with SetPolyG4; they advance halfword
 *           0x10/0x12/0x14 and word 0x08 there, so the seeded values are the
 *           start angle, start extent and start alpha of each quad.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The frame slot below the pointers is a reconstruction of a declaration the
 * original translation unit carried but this overlay's emitted code never
 * references: the original frame is 0x18 with vars=24 while this body's own
 * locals reserve 8 (the signed-magic division leaves one dead slot), it saves
 * no register, and no instruction in the original body reads or writes the
 * remaining 16 bytes (the prologue is a bare addiu $sp,-0x18, the epilogue a
 * bare addiu $sp,0x18). Declaring one 16-byte local restores that reservation
 * and measures 36/36 byte-exact; the name and type are a reconstruction choice
 * constrained only by the slot size. Same recorded shape as the frame slot in
 * seedWorkDistancePairArmCountdownAndAdvanceStepWorld02Area07714_801F4078 and
 * func_801F7670. Flagged for independent review; removing the declaration
 * leaves only the frame-size residual (33/36 instructions, 144 bytes).
 */
void seedThreeRotatingQuadRecordsWorld02Area10313_801F349C(void) {
  u32 unusedFrameSlot[4];
  u8* record;
  u8* work;
  u8 i;

  (void)unusedFrameSlot;
  record = PSX_PTR(u8, 0x800E4800u);
  i = 0u;
  work = D_1F800044;
  do {
    *(u32*)(record + 0x00) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x04) = *(u32*)(work + 0x38);
    *(u32*)(record + 0x08) = *(u32*)(work + 0x3C) + 0x3800000;
    *(u16*)(record + 0x10) = (u16)((i << 12) / 3);
    *(u16*)(record + 0x12) = 0xC00;
    *(u16*)(record + 0x14) = 0;
    i += 1u;
    record += 0x4BCu;
  } while (i < 3u);
}
