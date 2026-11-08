#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-reads the cell through
 * its own address load for every store group after the opening one, so only the
 * opening motion-word group, the flag access that shares the level-0x3E read and
 * the four closing stores reuse a single cell load.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Pointer cell of the shared work record whose byte 0x13D selects the 12-byte
 * record this step ramps down.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;
/* Pointer cell of the shared record whose halfword 0x0A this step consumes and
 * whose byte 0x01 it seeds when the scratch work record is released.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;
/* Base of the 12-byte records selected by the shared record byte 0x13D: the
 * record is based at 0x80145BD4 and the three bytes this step ramps down are its
 * +0x2, +0x3 and +0x4 fields, which the battle lifts write as an effect factor
 * (func_801DF7EC/func_801E38C0) and the area174/13 counter lift steps up. The
 * original folds the +1 and +2 displacements into this symbol's own address, so
 * the source keeps one base rather than the generated 0x80145BD7/0x80145BD8
 * rows.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];
/* Word published from the scratch work record's 0x3C field; func_801FB204
 * commits the same cell and hands its high halfword to func_80154FD8.
 * @source 0x80143F84 @kind data
 */
extern u32 D_80143F84;

void func_801B0DFC(void);
s32 func_80154F28(s32 x, s32 y);
void func_8014D6B8(u32 arg0);
void func_80154FD8(u32 arg0);

/* @source 0x801F44C4
 * @behavior Per-frame step of the scratch work record published through the
 * pointer cell at 0x1F800044. It adds the record's 0x20 step word to its 0x14
 * motion word, keeps that step word live for the shared mover func_801B0DFC
 * called immediately after, and then queries the shared distance routine
 * func_80154F28 with the record's coord_x_34/coord_y_38 pair, narrowing the
 * result to a signed halfword distance local for the comparison against the
 * record's signed halfword at 0x3E. When that 0x3E halfword is below the
 * narrowed distance the record is released: flag bit 0x20 of record byte 0 is
 * cleared, the 0x5C marker byte and the three level bytes 0x5D/0x5E/0x5F are
 * zeroed - each after its own fresh load of the cell - and one further load
 * then stores the raw result into the 0x3E halfword, zeroes the 0x14 and 0x20
 * words, writes 2 into the handler byte at 0x04, requests resource 0x39
 * through func_8014D6B8 and writes 4 into byte 0x01 of the shared record at
 * 0x80148210. Otherwise it consumes two units of that shared record's halfword
 * at 0x0A. Both arms continue into the shared tail, which subtracts 2 from the
 * three factor bytes +0x0, +0x1 and +0x2 of the 12-byte record selected by
 * byte 0x13D of the shared record at 0x80146250, each through a fresh read of
 * that cell; then, unless signed level byte 0x5D already reads 0x40, adds 4 to
 * signed level byte 0x5D and to unsigned level bytes 0x5E and 0x5F, the last
 * two each through a fresh load of the cell. It finishes by publishing the
 * record's 0x3C word into 0x80143F84 and handing the record's signed 0x3E
 * halfword to func_80154FD8. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/*
 * The opening step-word update reads the cell twice in one expression, so the
 * single load the original keeps is the compiler's own CSE and the step word
 * stays in the register the add leaves it in; naming that value in a local
 * costs allocation drift at the opening and closing store groups. The narrowed
 * halfword is its own s16 local because the original narrows the call result
 * into a second register before the comparison and keeps the raw result for
 * the 0x3E store; the world00/area03004 sibling calls the same coordinate-pair
 * result a distance, and func_80154F28 is the sibling of the func_8015477C
 * distance routine that lift uses. The factor triplet goes through an offset
 * local so the original's per-access re-materialisation of the table base
 * survives; writing the index inline instead makes the compiler cache the
 * whole address.
 */
void advanceScratchWorkAndSelectHandler2Past3eElseConsumeSharedThenStepFactorTripletAndRampLevelTripleWorld04Area17213_801F44C4(
    void) {
  s32 result;
  s16 distance;
  u32 offset;
  u8* record;

  *(s32*)(D_1F800044 + 0x14) += *(s32*)(D_1F800044 + 0x20);
  func_801B0DFC();
  result = func_80154F28(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38));
  distance = (s16)result;
  if (*(s16*)(D_1F800044 + 0x3e) < distance) {
    D_1F800044[0] &= 0xdf;
    D_1F800044[0x5c] = 0;
    D_1F800044[0x5d] = 0;
    D_1F800044[0x5e] = 0;
    D_1F800044[0x5f] = 0;
    record = D_1F800044;
    *(s16*)(record + 0x3e) = result;
    *(s32*)(record + 0x14) = 0;
    *(s32*)(record + 0x20) = 0;
    record[4] = 2;
    func_8014D6B8(0x39);
    ((u8*)D_80148210)[1] = 4;
  } else {
    D_80148210[5] -= 2;
  }
  offset = D_80146250[0x13d] * 12;
  *(u8*)(D_80145BD6 + offset) -= 2;
  offset = D_80146250[0x13d] * 12;
  *(u8*)(D_80145BD6 + offset + 1) -= 2;
  offset = D_80146250[0x13d] * 12;
  *(u8*)(D_80145BD6 + offset + 2) -= 2;
  if (*(s8*)(D_1F800044 + 0x5d) != 0x40) {
    *(s8*)(D_1F800044 + 0x5d) = *(s8*)(D_1F800044 + 0x5d) + 4;
    D_1F800044[0x5e] += 4;
    D_1F800044[0x5f] += 4;
  }
  D_80143F84 = *(u32*)(D_1F800044 + 0x3c);
  func_80154FD8(*(s16*)(D_1F800044 + 0x3e));
}
