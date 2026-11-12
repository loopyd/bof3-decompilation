#include "bof3/bof3.h"

/* First main-RAM flag bank this handler probes: bit 0x2C of the base at
 * 0x80144EB8 is the gate that has to be clear before the trigger table is
 * searched. No other lift in this repository names this base, so it stays
 * address-anchored. */
/* @source 0x80144EB8 @kind data */
extern u8 D_80144EB8;

/* Shared main-RAM flag bank the matched record writes into. Its bit 0x1C is
 * probed before the write, and the matched record's fourth byte names the bit
 * that is set. */
/* @source 0x80144F28 @kind data */
extern u8 D_80144F28;

/* Leading byte of the main-RAM game-mode record table, the third field the
 * trigger records are matched against. */
/* @source 0x80145E98 @kind data */
extern u8 D_80145E98;

/* The two-record trigger table this payload carries at 0x801F6B14, addressed as
 * one byte symbol per field so each of the three record reads keeps its own
 * address pair, plus the fourth byte of the matched record. Two 4-byte records
 * sit at 0x801F6B14 and 0x801F6B18, both inside the D_801F63C8 textbin run:
 * record 0 reads 0x33/0x1D/0x07/0x05 and record 1 reads 0x19/0x1D/0x07/0x06. */
/* @source 0x801F6B14 @kind table */
extern u8 D_801F6B14[];
/* @source 0x801F6B15 @kind table */
extern u8 D_801F6B15[];
/* @source 0x801F6B16 @kind table */
extern u8 D_801F6B16[];
/* @source 0x801F6B17 @kind table */
extern u8 D_801F6B17[];

/* Shared flag services of the main executable: tester and setter. */
/* @source 0x8015B5D4 */
extern s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x8015B5F4 */
extern void func_8015B5F4(u32 arg0, s32 arg1);

/* Main-exe sound cue dispatcher at 0x8015DF18; the sibling lift
 * setTriggerRecordFlagAndQueueCue203OnMatchedArgumentsWorld03Area11813_801F502C
 * reaches the same jal word as dispatchSoundCue. */
/* @source 0x8015DF18 */
extern void func_8015DF18(u16 cue);

/* Main-exe routine this overlay calls with 0xF on the accepted path. */
/* @source 0x8019ABB0 */
extern void func_8019ABB0(s32 arg0);

/* @source 0x801F5880
 * @behavior Two-stage trigger handler. It first probes bit 0x2C of the shared
 *   main-RAM byte bank 0x80144EB8 through the shared flag tester func_8015B5D4
 *   and returns 0 while that bit is set. Otherwise it matches the two
 *   zero-extended word arguments and the shared game-mode byte 0x80145E98
 *   against the two records of the trigger table this payload carries at
 *   0x801F6B14: a record is accepted when its first two bytes equal the two
 *   arguments and its third byte equals the game-mode byte, and the search
 *   walks the table by a byte offset advanced 0x04 per record, leaving the index
 *   at 2 when nothing is accepted. An exhausted search returns 0; so does the
 *   accepted path once the shared flag bank 0x80144F28 reports bit 0x1C through
 *   the same tester. Only on the accepted and still-unset path does the matched
 *   record's fourth byte go to the shared setter func_8015B5F4 as the bit to set
 *   in 0x80144F28, after which the handler queues sound cue 0x201 through the
 *   main-exe dispatcher 0x8015DF18, calls the main-exe routine 0x8019ABB0 with
 *   0xF and returns 1.
 *
 *   No jal and no word of this payload names 0x801F5880, and the main executable
 *   holds neither a call nor a pointer word for it, so the caller and both
 *   argument roles stay structural.
 *
 *   Shape: the 0x20-byte frame keeps $s0/$s1/$s2/$ra at 0x10/0x14/0x18/0x1C and
 *   agrees with the declared 0x11C boundary. The arguments arrive as words and
 *   survive the gate call in $s1/$s2, so the source keeps them there and takes
 *   the byte extracts as separate locals after the call: masking the arguments
 *   in place leaves the masks on $s1/$s2 and pushes the mode address and the
 *   record displacement down to $a1/$a0, while the separate locals keep them on
 *   $a0/$a1 and leave $a3/$a2 for the game-mode address and the displacement.
 *   The game-mode address is taken once into a pointer whose byte stays loaded
 *   inside the loop, and the statement order is i = 0, byte extracts, mode
 *   address, displacement. The search is a for over the displacement with the
 *   continue test `i < 2` after the increment, and the accepted record is
 *   reached through a break, so the exit test stays the single bottom `slti`.
 *   The cue precedes the 0xF call.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setMatchedTriggerRecordFlagAndQueueCue201WhenSharedFlag2cClearWorld02Area07714_801F5880(
    s32 arg0, s32 arg1) {
  s32 i;
  s32 off;
  u32 id0;
  u32 id1;
  u8* mode;

  if (func_8015B5D4((u32)&D_80144EB8, 0x2C) != 0) {
    return 0;
  }
  i = 0;
  id0 = arg0 & 0xFF;
  id1 = arg1 & 0xFF;
  mode = &D_80145E98;
  off = 0;
  for (; i < 2; i++, off += 4) {
    if (id0 == D_801F6B14[off] && id1 == D_801F6B15[off] &&
        D_801F6B16[off] == *mode) {
      break;
    }
  }
  if (i == 2) {
    return 0;
  }
  if (func_8015B5D4((u32)&D_80144F28, 0x1C) != 0) {
    return 0;
  }
  func_8015B5F4((u32)&D_80144F28, D_801F6B17[i * 4]);
  func_8015DF18(0x201);
  func_8019ABB0(0xF);
  return 1;
}
