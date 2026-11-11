#include "bof3/bof3.h"

/* Shared main-RAM flag bank. The matched trigger record names the bit this
 * function sets through the shared setter below; the World03/Area118 lift
 * measured the same bank and setter. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Leading byte of the main-RAM game-mode record table, the third field a
 * trigger record is matched against. */
extern u8 D_80145E98; /* @source 0x80145E98 @kind data */

/* The two-record trigger table this payload carries at the tail of the data
 * span 0x801F442C, addressed as one byte symbol per field so each of the three
 * record reads keeps its own address pair, plus the fourth byte of the matched
 * record. Record 0 reads 0x5A/0x0A/0x01/0x5C and record 1 reads
 * 0x3E/0x42/0x07/0x5D. */
extern u8 D_801F45E4[]; /* @source 0x801F45E4 @kind data */
extern u8 D_801F45E5[]; /* @source 0x801F45E5 @kind data */
extern u8 D_801F45E6[]; /* @source 0x801F45E6 @kind data */
extern u8 D_801F45E7[]; /* @source 0x801F45E7 @kind data */

/* Shared flag services and cue dispatcher of the main executable, outside this
 * overlay's payload; Splat binds them through this target's generated
 * undefined-funcs map, so no target-map row is needed. */
void func_8015B5F4(u32 arg0, s32 arg1);
void func_8015DF18(u16 cue);

/* @source 0x801F4364
 * @behavior Matches the two zero-extended word arguments and the shared
 *           game-mode byte 0x80145E98 against the records of the two-entry
 *           trigger table this payload carries at 0x801F45E4. A record is
 *           accepted when its first byte equals the first argument, its second
 *           byte equals the second argument, and the low nibble of its third
 *           byte equals the game-mode byte, which the original masks with 0xF.
 *           The search walks the table by a byte offset advanced 0x04 per
 *           record and stops at the accepted record; the accepted record then
 *           sets its own fourth byte as a bit of the shared main-RAM flag bank
 *           0x80144F28 through the shared setter func_8015B5F4 and queues sound
 *           cue 0x204 through the resident cue dispatcher 0x8015DF18, and that
 *           path returns 1. A run that reads the last record without
 *           acceptance returns 0 and calls nothing.
 *
 *           No jal and no word of this payload names the address, and the main
 *           executable holds neither a call nor a pointer word for it, so the
 *           caller and both argument roles stay structural and the name keeps
 *           them.
 *
 *           Name evidence: the record layout, the shared flag-bank setter and
 *           the resident cue dispatcher are the ones the exact World03/Area118
 *           lift of the same body family at 0x801F502C measured, where the
 *           fourth record byte is the bank bit and 0x8015DF18 is the sound cue
 *           dispatcher; this body carries the same table, setter and cue call
 *           and only omits that sibling's flag tester and its 0xF extra call.
 *
 *           Shape: the 0xC8 boundary holds the frame, the single loop and the
 *           shared epilogue with the masked arguments in place and the return-0
 *           block ahead of the epilogue. A masked little-endian scan of every
 *           payload under out/binaries (all 16-bit immediates and jump targets
 *           cleared) finds this instruction shape only at 0x801F4364 of this
 *           payload, so no byte-identical sibling exists and clone was
 *           declined; the World03/Area118 source above supplied the statement
 *           order, the byte-offset walk and the i * 4 record read, and this
 *           body's own literals, table bytes and callee bindings come from the
 *           original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setTriggerRecordFlagAndQueueCue204OnMatchedArgumentsWorld03Area13913_801F4364(
    s32 arg0, s32 arg1) {
  s32 i;
  s32 off;
  u8* mode;

  i = 0;
  arg0 &= 0xFF;
  arg1 &= 0xFF;
  mode = &D_80145E98;
  off = 0;
  for (; i < 2; i++, off += 4) {
    if (arg0 == D_801F45E4[off] && arg1 == D_801F45E5[off] &&
        (D_801F45E6[off] & 0xF) == *mode) {
      break;
    }
  }
  if (i == 2) {
    return 0;
  }
  func_8015B5F4((u32)&D_80144F28, D_801F45E7[i * 4]);
  func_8015DF18(0x204);
  return 1;
}
