#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 for every access group below, one
 * group per statement, so the cell is its own fixed-address symbol rather than
 * the constant-address accessor, whose shared base would stay live across the
 * calls and change the allocation. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Overlay quad renderer of this area at 0x801F38E0, still an asm boundary in
 * this payload, so it keeps its address anchor. It projects the three-word
 * point handed as arg0 through the shared point projector func_801AFF04,
 * passes the signed extent handed as arg1 as a size pair to the shared
 * size-pair converter func_801AFFD8, and draws one semi-transparent four-vertex
 * primitive through g_PrimCursor whose colour bytes come from the channel mask
 * handed as arg2. */
void func_801F38E0(void* point, s16 extent, s32 channels);

/* Effect entry seeder at 0x801F4394, still an asm boundary in this payload. It
 * copies the work object's 0x34/0x38/0x3C position words into the 0x2C-byte
 * entry handed in $a0 and seeds the rest of that entry from the shared angular
 * pair and the work record. */
void func_801F4394(u8* entry);

/* Shared EXE-side cue dispatcher; the constant argument seeds a caller's
 * channel refresh, so the call-site width is the narrow constant form. */
void func_8015DF18(u16 arg0);

/* Clears the lead byte of the 0x18-byte-stride record table at 0x800E4800. */
void clearWorkTableByte0At800E4800World04Area18813_801F3DB4(void);

/* @source 0x801F3484
 * @behavior Overlay progress step handler, element 3 of the nine-word handler
 * run D_801F4E04 that the dispatcher 0x801F32C0 indexes with the unsigned work
 * byte at offset +0x1 of the work record published at the scratchpad pointer
 * cell 0x1F800044, called with no arguments and no result consumed. It advances
 * the record's origin word at +0x6C by 0xC0000, subtracts 6 from the unsigned
 * halfword at +0x2E and stores the truncated halfword back, and renders the
 * record through the overlay renderer 0x801F38E0 with the three-word point at
 * +0x64, the new halfword read as a signed halfword and the channel mask 6, so
 * the shape drawn at that origin narrows by 6 every frame it runs. It then
 * ticks the countdown byte at +0x09 down by one; while that byte stays
 * non-zero the handler returns. On the frame it reaches zero it re-arms the
 * countdown with 0x20, clears the frame byte at +0x06, advances the work byte
 * at +0x1 by one, clears the lead byte of the 0x18-byte-stride record table at
 * 0x800E4800, copies the origin triple at +0x64, +0x68 and +0x6C into the
 * position triple at +0x34, +0x38 and +0x3C, re-seeds the first 0x10 entries of
 * the 0x2C-byte-stride effect table at 0x800E48C0 through 0x801F4394, and
 * queues the front-end cue 0x215 through 0x8015DF18. It takes no arguments,
 * returns nothing and its 0x20-byte frame keeps $ra at 0x18, the effect-entry
 * cursor at 0x14 and the entry counter at 0x10 across the calls.
 *
 * Evidence: the word at 0x801F4E10 inside the nine-word code-pointer run
 * 0x801F4E04..0x801F4E28 names this address, and that run is the handler table
 * this target's exact area18813DispatchTables.c binds as
 * `void (*D_801F4E04[])(void)` and indexes with the work byte at +0x1 through
 * its exact dispatcher dispatchStateByte1Table4e04World04Area18813_801F32C0,
 * so the run names this function's position among the overlay's progress
 * handlers. The original bytes at 0x801F3484 are the head `lui 0x1F80` plus
 * lw 0x44, addiu $sp,$sp,-0x20 with sw $ra,0x18, $s1,0x14 and $s0,0x10, lw
 * 0x6C and lhu 0x2E off that one materialisation, addiu $a2,$zero,0xC0000
 * consumed by the origin-word add, addiu $a1,-0x6, sh 0x2E, the sll/sra pair
 * that sign-extends it for the call, jal 0x801F38E0 with the origin addiu into
 * $a0 and the origin-word store in its delay slot, then the countdown group as
 * its own lui/lw pair, lbu 0x9, addiu -0x1, sb 0x9, andi 0xFF, bnez to the
 * shared epilogue with addiu $v0,$zero,0x20 in its delay slot. The guard block
 * carries one lui/lw pair per statement - sb 0x20 to +0x9, sb $zero to +0x6,
 * and lbu 0x1 plus addiu 1 plus sb 0x1 in the delay slot of jal 0x801F3DB4 -
 * then one more pair for the triple copy, lt/sltiu-bounded effect-entry walk
 * `addiu $s0,1`, `andi $v0,$s0,0xFF`, `sltiu $v0,$v0,0x10` and bnez with
 * `addiu $s1,$s1,0x2C` in its delay slot, and jal 0x8015DF18 with addiu
 * $a0,$zero,0x215.
 *
 * Field roles: the 0x34/0x38/0x3C triple is the record's position, which the
 * exact claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0 seeds a
 * spawned record from and which the exact neighbour lift
 * seedWorkWords18To20FromOriginAndSetStepWordsWhenCountdownExpiresThenAdvanceWorkByte1World04Area18813_801F3598
 * rewinds from the 0x64/0x68/0x6C origin triple, so that triple is the origin.
 * The 0x2E halfword is the drawn extent: the exact sibling
 * growWorkExtentAndDrawShapeThenOnCountdownExpiryArm80QueueCue214AndAdvanceWorkByte1World04Area18813_801F3368
 * of the same handler run passes it to the same renderer 0x801F38E0 as arg1
 * after adding 0x20. 0x800E48C0 is 0x800E4800 + 8 * 0x18, the 0x2C-byte-stride
 * effect table the exact area17213 lift
 * drawWorkQuadReseedEffectEntriesAtProgress0aThenArmCountdownAndAdvanceStateWorld04Area17213_801F30F8
 * walks with the same counter and cursor shape, so this body re-seeds that
 * table's first 0x10 entries.
 *
 * Shape evidence: the head, the countdown group and the guard tail are the
 * statement sequence of the exact cross-target twin
 * advanceWorkHalfword2EAndOriginZInvokeOriginHelperTickCountdownReload32ThenResetRecordTableScenarioSce15ef100_801D0DD8,
 * written with this body's own -6 extent step, 0xC0000 origin word step, 0x20
 * re-arm and 0x215 cue; that lift records that writing the origin-word update
 * before the halfword advance puts its load ahead of the `lhu` and lets the
 * delay-slot filler place that store in the `jal` slot, which is this body's
 * order. The countdown group and the re-arm are the statement sequence of the
 * exact same-target sibling 0x801F3368, whose `func_8015DF18` cue call after
 * `D_1F800044[1]++` is the shape that puts the state-byte store in the jal
 * delay slot here. The effect-entry walk is the statement sequence of the
 * exact area17213 lift named above, with this body's 0x10 bound.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatWorkExtentAndAdvanceOriginZDrawThenOnCountdownExpiryArm20ReseedEffectEntriesQueueCue215AndAdvanceWorkByte1World04Area18813_801F3484(
    void) {
  u8* work;
  u8* motion;
  u8* entry;
  u16 advance;
  u8 index;
  s32 originX;
  s32 originY;
  s32 originZ;

  work = D_1F800044;
  *(u32*)(work + 0x6C) += 0xC0000;
  advance = *(u16*)(work + 0x2E) - 6;
  *(u16*)(work + 0x2E) = advance;
  func_801F38E0(work + 0x64, (s16)advance, 6);
  if (--D_1F800044[9] == 0) {
    D_1F800044[9] = 0x20;
    D_1F800044[6] = 0;
    D_1F800044[1]++;
    clearWorkTableByte0At800E4800World04Area18813_801F3DB4();
    motion = D_1F800044;
    originX = *(s32*)(motion + 0x64);
    originY = *(s32*)(motion + 0x68);
    originZ = *(s32*)(motion + 0x6C);
    *(s32*)(motion + 0x34) = originX;
    *(s32*)(motion + 0x38) = originY;
    *(s32*)(motion + 0x3C) = originZ;
    entry = PSX_PTR(u8, 0x800E48C0u);
    index = 0u;
    do {
      func_801F4394(entry);
      entry += 0x2Cu;
      index += 1u;
    } while (index < 0x10u);
    func_8015DF18(0x215);
  }
}
