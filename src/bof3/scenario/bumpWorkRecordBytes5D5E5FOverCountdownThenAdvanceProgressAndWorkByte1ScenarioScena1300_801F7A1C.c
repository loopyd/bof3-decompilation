#include "bof3/bof3.h"

/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* @source 0x801468E5 @kind unknown */
extern u8 D_801468E5[];

/* @source 0x801468E6 @kind unknown */
extern u8 D_801468E6[];

/* @source 0x801468E7 @kind unknown */
extern u8 D_801468E7[];

/* @source 0x801F7A1C
 * @behavior Entry 10 of this overlay's twelve-word work-object handler run based
 * at 0x801FBEF0 (its word at 0x801FBF18), the run the frame dispatcher
 * func_801F6EA4 selects with byte 0x01 of the scratchpad work object published
 * at pointer slot 0x1F800044. It takes no arguments and returns nothing, and it
 * branches on that object's byte at 0x09: while the byte is non-zero it adds
 * +0x8 to each of the three bytes at 0x5D/0x5E/0x5F of the two 0x98-byte work
 * records 0x80146888[byte 0x03] and 0x80146888[byte 0x04], six accesses whose
 * bases are the addresses 0x801468E5, 0x801468E6 and 0x801468E7 (0x80146888 +
 * 0x5D, one per slot, and the third byte named as the next address), and then
 * stores that byte minus one through a freshly loaded pointer. When byte 0x09
 * reads zero it increments the shared scenario progress byte at 0x80146864 and
 * stores 3 into the object's dispatch byte 0x01, so the next frame runs
 * 0x801FBEF0[3] = 0x801F71DC. The immediately preceding run entry 9
 * (0x801F7958) spends the same countdown converging what the run entry 8
 * (0x801F763C) opened, and arms it with 0xF.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The six bumps each re-read the scratchpad pointer cell before they touch the
 * record, so the cell load is written once per group: the entry load keeps the
 * object pointer in $a0 across both arms, the five later bump loads land in $v0,
 * and the countdown group's own reload lands in $v1. The cell address is formed
 * through the named `slot_offset` local for the reason the run's entry 9 records:
 * a constant-address cell keeps one hoisted address across its reads, where the
 * original emits `lui $v0,0x1F80` plus `lw $v0,0x44($v0)` at each read. The
 * record byte is reached as a byte-scaled `slot * 152` offset from the respective
 * 0x801468E5/E6/E7 base, which keeps the original's folded `lui $at,%hi(sym)` /
 * `addu $at,$at,$v0` / `lbu|sb %lo(sym)($at)` pair for each of the twelve
 * accesses.
 *
 * The scenario progress byte is reached through an address local, `progress =
 * &g_ScenarioProgress`, rather than as a bare scalar: a direct `g_ScenarioProgress
 * += 1` splits the symbol into `lui` plus a displacement at each of the two
 * accesses, while the address local is what keeps the original's single
 * `lui $v0,%hi` / `addiu $v0,$v0,%lo` base with `lbu $v1,0($v0)` and
 * `sb $v1,0($v0)` on it.
 */
void bumpWorkRecordBytes5D5E5FOverCountdownThenAdvanceProgressAndWorkByte1ScenarioScena1300_801F7A1C(void) {
  u32 slot_offset;
  u8 *work;
  u8 *stage;
  u8 *countdown;
  u8 *progress;
  u8 slot;
  s32 offset;

  slot_offset = 0x44u;
  work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
  if (work[9] != 0) {
    slot = work[3];
    offset = slot * 152;
    D_801468E5[offset] += 8;
    stage = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    slot = stage[3];
    offset = slot * 152;
    D_801468E6[offset] += 8;
    stage = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    slot = stage[3];
    offset = slot * 152;
    D_801468E7[offset] += 8;
    stage = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    slot = stage[4];
    offset = slot * 152;
    D_801468E5[offset] += 8;
    stage = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    slot = stage[4];
    offset = slot * 152;
    D_801468E6[offset] += 8;
    stage = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    slot = stage[4];
    offset = slot * 152;
    D_801468E7[offset] += 8;
    countdown = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    countdown[9] = countdown[9] - 1;
  } else {
    progress = &g_ScenarioProgress;
    *progress = *progress + 1;
    work[1] = 3;
  }
}
