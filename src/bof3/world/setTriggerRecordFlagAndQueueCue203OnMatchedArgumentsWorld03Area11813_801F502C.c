#include "bof3/bof3.h"

/* Shared main-RAM flag bank. Its bit 0x1C is the gate this function probes and
 * the matched trigger record names the bit it sets; the game00 lift clears the
 * same bit through func_8015B5A8 and this overlay's neighbours probe their own
 * bank through the tester declared below. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Leading byte of the main-RAM game-mode record table, the third field a
 * trigger record is matched against. */
extern u8 D_80145E98; /* @source 0x80145E98 @kind data */

/* The two-record trigger table this payload carries at 0x801F59E0, addressed
 * as one byte symbol per field so each of the three record reads keeps its own
 * address pair, plus the fourth byte of the matched record. Record 0 reads
 * 0x0B/0x46/0x01/0x85 and record 1 is all zeroes, the empty terminator. */
extern u8 D_801F59E0[];
extern u8 D_801F59E1[];
extern u8 D_801F59E2[];
extern u8 D_801F59E3[];

/* Shared flag services of the main executable: tester and setter. */
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B5F4(u32 arg0, s32 arg1);

/* Main-executable routine this overlay calls once, with 0xF, before the cue. */
void func_8019ABB0(s32 arg0);

/* @source 0x801F502C
 * @behavior Matches the two zero-extended word arguments and the shared
 *   game-mode byte 0x80145E98 against the records of the two-entry trigger
 *   table this payload carries at 0x801F59E0. A record is accepted when its
 *   first three bytes equal the two arguments and the game-mode byte, and the
 *   search walks the table by a byte offset advanced 0x04 per record, stopping
 *   at the accepted record. The accepted record then sets its own fourth byte
 *   as a bit of the shared main-RAM flag bank 0x80144F28 through the shared
 *   setter func_8015B5F4, unless that bank already reports bit 0x1C through the
 *   shared tester func_8015B5D4; only on that path does it call the
 *   main-executable routine 0x8019ABB0 with 0xF and queue sound cue 0x203
 *   through the resident cue dispatcher, and only on that path does it return
 *   1. A run that reaches the end of the table without acceptance (the
 *   displacement is then 1, read as the same access pattern as the terminator
 *   record), the all-zero terminator record itself and an already-set bit 0x1C
 *   all return 0.
 *
 *   No jal and no word of this payload names the address, and the main
 *   executable holds neither a call nor a pointer word for it, so the caller
 *   and both argument roles stay structural and the name keeps them.
 *
 *   Shape: the zero-byte surplus over the 0xF0 row and the 0x20-byte frame that
 *   keeps $s0/$s1/$ra at 0x10/0x14/0x18 with the record pointer cell untouched
 *   agree with the declared boundary. The observed form needs the two
 *   arguments masked in place at entry (so they arrive as words), the game-mode
 *   address taken once into a pointer whose byte stays loaded inside the loop,
 *   and the source statement order i = 0, masks, mode address, offset - the
 *   mask statements before the address taking is what keeps `andi a0/a1` ahead
 *   of the `lui/addiu a3` pair. The loop is a for over the displacement with the
 *   continue test `i <= 0` after the increment, and the accepted record is
 *   reached through a break, so the exit test stays the single bottom `blez`
 *   the original shows rather than the peeled form a do-while body produces.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setTriggerRecordFlagAndQueueCue203OnMatchedArgumentsWorld03Area11813_801F502C(
    s32 arg0, s32 arg1) {
  s32 i;
  s32 off;
  u8* mode;

  i = 0;
  arg0 &= 0xFF;
  arg1 &= 0xFF;
  mode = &D_80145E98;
  off = 0;
  for (; i <= 0; i++, off += 4) {
    if (arg0 == D_801F59E0[off] && arg1 == D_801F59E1[off] &&
        D_801F59E2[off] == *mode) {
      break;
    }
  }
  if (i == 1) {
    return 0;
  }
  if (func_8015B5D4((u32)&D_80144F28, 0x1C) != 0) {
    return 0;
  }
  func_8015B5F4((u32)&D_80144F28, D_801F59E3[i * 4]);
  func_8019ABB0(0xF);
  dispatchSoundCue(0x203);
  return 1;
}
