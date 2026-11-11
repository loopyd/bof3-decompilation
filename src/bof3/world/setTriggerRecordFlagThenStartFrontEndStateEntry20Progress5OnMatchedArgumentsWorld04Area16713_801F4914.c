#include "bof3/bof3.h"

/* Shared main-RAM flag bank. The matched trigger record names the bit this
 * function sets through the shared setter below; the exact World03/Area118 and
 * World03/Area139 lifts of the same body family set their own record bit in the
 * same bank 0x80144F28, and this target's own setHalfword932c980StateEntry20
 * Progress1E lift probes it through the sibling tester 0x8015B5D4. */
extern u8 D_80144F28[]; /* @source 0x80144F28 @kind data */

/* Leading byte of the main-RAM game-mode record table, the third field a
 * trigger record is matched against. */
extern u8 D_80145E98; /* @source 0x80145E98 @kind data */

/* The single-record trigger table this payload carries in the last word of its
 * data span at 0x801F68B4, addressed as one byte symbol per field so each of
 * the three record reads keeps its own address pair, plus the fourth byte of
 * the matched record. The record reads 0x2A/0x34/0x07/0x47, and the loop bound
 * 1 of the original says the table holds exactly this one record: a second
 * record at +0x04 would run past the end of the payload. */
extern u8 D_801F68B4[]; /* @source 0x801F68B4 @kind data */
extern u8 D_801F68B5[]; /* @source 0x801F68B5 @kind data */
extern u8 D_801F68B6[]; /* @source 0x801F68B6 @kind data */
extern u8 D_801F68B7[]; /* @source 0x801F68B7 @kind data */

/* State entry byte and progress byte of the shared front-end tuple in the main
 * executable, outside this overlay's payload. Every request handler of this
 * target writes them as a pair; the exact sibling
 * setStatusFFFFAndModeByte1Or2ByGateThenSeedCounter40AndStateEntry20Progress15
 * World04Area16713_801F3CE0 names the same two bytes. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */
extern u8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared flag setter of the main executable, outside this overlay's payload:
 * sets the bit its second argument selects in the bank its first argument
 * addresses. The exact World03/Area118 and World03/Area139 lifts of this body
 * family declare the same prototype and pass the same bank. */
extern void func_8015B5F4(u32 arg0, s32 arg1); /* @source 0x8015B5F4 */

/* Shared front-end startup helper of the main executable, outside this
 * overlay's payload. The exact sibling setStatusFFFFAndModeByte0Or1ByGateThen
 * SeedCounter40AndStateEntry20Progress14World04Area16713_801F3DD4 declares it
 * with the same void signature and runs it immediately before its own state
 * entry and progress stores. */
extern void func_8015C088(void); /* @source 0x8015C088 */

/* @source 0x801F4914
 * @behavior Matches the two zero-extended word arguments and the shared
 *           game-mode byte D_80145E98 against the records of the one-record
 *           trigger table this payload carries at the tail of its data span,
 *           0x801F68B4. A record is accepted when its first byte equals the
 *           first argument, its second byte equals the second argument, and the
 *           low nibble of its third byte equals the game-mode byte, which the
 *           original masks with 0xF. The search walks the table by a byte
 *           offset advanced 0x04 per record and stops at the accepted record.
 *           The accepted record then sets its own fourth byte, 0x47, as a bit
 *           of the shared main-RAM flag bank D_80144F28 through the shared
 *           setter func_8015B5F4, runs the shared front-end startup helper
 *           func_8015C088, requests shared state entry 0x20 through
 *           D_801448EB with progress 5 in D_801448EC and returns 1. A run that
 *           reaches the end of the table without acceptance returns 0 and
 *           calls nothing. The fourth argument byte is spent on the setter
 *           call alone, so this body's flag bit enters no test.
 *
 *           No jal and no word of this payload names the address, and the
 *           payload carries no pointer word for it anywhere in its data span,
 *           so the caller and both argument roles stay structural and the name
 *           keeps them.
 *
 *           Name evidence: the record layout, the low-nibble mode test, the
 *           shared flag-bank setter and the one-byte-per-field table walk are
 *           the ones the exact World03/Area118 lift 0x801F502C and the exact
 *           World03/Area139 lift 0x801F4364 measured for this body family, and
 *           both name the accepted record a trigger record. func_8015C088 and
 *           the D_801448EB/D_801448EC pair carry the names this target's own
 *           exact siblings give the front-end startup helper and the state
 *           entry and progress bytes.
 *
 *           Shape: both World03 lifts measure this exact loop text, and this
 *           body is the World03/Area118 form - a for over the displacement with
 *           the continue test `i <= 0` after the increment and the accepted
 *           record reached through a break, which keeps the single bottom
 *           `blez` the original shows - with the World03/Area139 record count
 *           of one and its `(record & 0xF) == mode` operand order, and its own
 *           tail in place of their flag tester plus cue: the setter call, the
 *           front-end startup call and the two state stores, with no tester
 *           gate and no cue. A masked little-endian scan of the first 0x20 and
 *           the first 0x40 bytes of this span over every payload under
 *           out/binaries finds the World03/Area139 span at its own 0x1764
 *           offset, which differs from this one in its table address, its
 *           record count and its whole tail, so no byte-identical or
 *           literal-only sibling exists; clone is declined and the statements
 *           are written from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setTriggerRecordFlagThenStartFrontEndStateEntry20Progress5OnMatchedArgumentsWorld04Area16713_801F4914(
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
    if (arg0 == D_801F68B4[off] && arg1 == D_801F68B5[off] &&
        (D_801F68B6[off] & 0xF) == *mode) {
      break;
    }
  }
  if (i == 1) {
    return 0;
  }
  func_8015B5F4((u32)&D_80144F28, D_801F68B7[i * 4]);
  func_8015C088();
  D_801448EB = 0x20;
  D_801448EC = 5;
  return 1;
}
