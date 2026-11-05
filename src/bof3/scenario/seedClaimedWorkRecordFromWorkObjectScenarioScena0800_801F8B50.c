#include "bof3/bof3.h"
#include <rand.h>

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F8B50
 * @behavior Seeds the work record it receives in $a0: it stores 1 in the
 * record's claim byte at offset 0, 0 in its state byte at offset 1, 8 in its
 * countdown byte at offset 2, 0 in its byte at offset 3, 0x40 in its re-arm
 * byte at offset 4 and clears its word at offset 8. It then copies the
 * overlay's scratch work object - the object published by the pointer cell at
 * 0x1F800044 - into the record's position triple: word 0x0C takes the work
 * word 0x34, word 0x10 the work word 0x38, and word 0x14 the work word 0x3C
 * unchanged, while the two scattered words add ((rand() & 0xFF) << 11) and
 * subtract 0x40000, so each lands between 0x40000 below and 0x3F800 above the
 * work word: a random jitter of up to about four of the 0x10000-per-unit steps
 * these position words use. The scratch work pointer is read once per scattered
 * word, so the object is picked up afresh after each rand() call; the second
 * read lands in $a0 because $v1 carries the first position result out through
 * the jal delay slot. It takes one argument, returns nothing, and its
 * 0x20-byte frame saves $ra and $s0 for the record pointer plus $s1 for the
 * -0x40000 constant both scatter arms add. No other memory is read or written.
 *
 * Name evidence: this target's findFreeWorkRecordScenarioScena0800_801F8B14
 * scans the 0x8 entries of the 0x1C-byte-stride record table at 0x800E4800 for
 * the first entry whose lead byte reads zero, and the only call site of this
 * address in the payload - jal 0x801F8B50 at 0x801F7A44 inside func_801F79EC -
 * is reached only when that scan returned a non-zero record, which it then
 * passes here in $a0. The same neighbour records the field roles this function
 * writes: byte 0 is the claim flag, byte 1 the state byte the work-byte-1
 * dispatchers select handlers with, byte 2 the countdown byte that
 * tickCountdownAdvanceWorkByte3ReloadFromByte4AndAdvanceStateScenarioScena0800_801F8CDC
 * decrements and byte 4 the re-arm value that same handler reloads. The
 * 0x8-entry/0x1C-stride table at 0x800E4800 and the scratch work object at
 * pointer slot 0x1F800044 are the ones this target's exact siblings use. This
 * target's sources never localize the 0x40000 scatter step or the work
 * position triple, so those roles stay described by offset.
 *
 * Shape evidence: bytes 0x1F50..0x1FF7 of this target's payload are
 * byte-identical to bytes 0x17D0..0x1877 of emi/scenario/scena07/00, where the
 * same function is still the unlifted asm span func_801F83D0 with the same
 * single caller (jal 0x801F83D0 at 0x801F6EC4 inside func_801F6E90 there).
 * That copy is not an exact lift, so it is shape evidence only.
 *
 * Matching note: the decisive shape is the shifted random factor. Masking and
 * shifting rand()'s low byte into its own local before the scratch pointer is
 * read keeps the scale in $v0 and lets the pointer load land after the call;
 * folding the same mask and shift into the position expression instead keeps
 * the pointer live across rand(), which adds $s2 and moves $ra from 0x18 to
 * 0x1C in the same 0x20-byte frame. One pointer local per scattered word, each
 * assigned after its own call, keeps the two reads in the registers the
 * original uses ($v1 then $a0).
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedClaimedWorkRecordFromWorkObjectScenarioScena0800_801F8B50(u8* record) {
  s32 scatter;
  u8* work;
  u8* next;

  record[0] = 1;
  record[2] = 8;
  record[1] = 0;
  record[3] = 0;
  *(u32*)(record + 0x8) = 0;
  record[4] = 0x40;
  scatter = (rand() & 0xFF) << 11;
  work = scenarioScena0800Work;
  *(u32*)(record + 0xC) = *(u32*)(work + 0x34) + scatter - 0x40000;
  scatter = (rand() & 0xFF) << 11;
  next = scenarioScena0800Work;
  *(u32*)(record + 0x10) = *(u32*)(next + 0x38) + scatter - 0x40000;
  *(u32*)(record + 0x14) = *(u32*)(next + 0x3C);
}
