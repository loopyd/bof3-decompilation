#include "bof3/bof3.h"

/* Word +0x34 of the 0x140-byte-stride local work records based at 0x80145E90:
 * the first word of the record's position pair. Addressed here through a byte
 * offset so both words keep their own symbol base, which is the form the
 * original shows.
 * @source 0x80145EC4 @kind unknown
 */
extern u8 D_80145EC4[];
/* Word +0x38 of the same work records: the second word of the position pair.
 * @source 0x80145EC8 @kind unknown
 */
extern u8 D_80145EC8[];

/* Six-byte bounds records of this overlay, inside the payload data span based at
 * 0x801F4624. Byte +0 and byte +2 bound the +0x34 word, byte +1 and byte +3
 * bound the +0x38 word, and bytes +4/+5 are the two ids the caller publishes for
 * the matching record. Each bound byte is the integer part of a 16.16 value, so
 * it is compared after a 16-bit shift. Record 0 holds 0x26/0x19/0x27/0x24 with
 * kind 0x01 and flag id 0x85.
 * @source 0x801F558C @kind table
 */
extern u8 D_801F558C[];
extern u8 D_801F558D[];
extern u8 D_801F558E[];
extern u8 D_801F558F[];

/* @source 0x801F4110
 * @behavior Overlay query: reads the two position words (+0x34 and +0x38) of the
 *           0x140-byte-stride local work record selected by the byte argument,
 *           then walks the six-byte bounds records of the table at 0x801F558C
 *           and returns the index of the first record whose byte bounds contain
 *           both words, or 0xFF when none does. Only the first record is ever
 *           visited: the counter is compared against zero, so the shipped loop
 *           runs once. Takes one byte argument, uses no stack frame and makes no
 *           calls.
 *
 *           This is the World03/Area118 twin 0x801F4C08 of the same overlay
 *           family, proven equal by a payload compare of the two 0xC8 spans: the
 *           four differing words are the four `lbu %lo(<bound byte>)` table
 *           reads, each shifted by the same -0x408 (0x801F5994 -> 0x801F558C),
 *           so the two spans are one body and this one carries its own table
 *           base. The caller 0x801F41D8 (still an `asm` span of this target)
 *           passes its live record cursor masked to a byte and tests the result
 *           against 0xFF, which is the absent-record answer below.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findWorkPositionBoundsIndexWorld03Area11713_801F4110(u8 index) {
  s32 i;
  s32 record;
  s32 offset;
  s32 first;
  s32 second;

  i = 0;
  record = 0;
  offset = index * 0x140;
  first = *(s32 *)(D_80145EC4 + offset);
  second = *(s32 *)(D_80145EC8 + offset);
  for (; i <= 0; i++, record += 6) {
    if (first >= ((s32)D_801F558C[record] << 16) &&
        ((s32)D_801F558E[record] << 16) >= first &&
        second >= ((s32)D_801F558D[record] << 16) &&
        ((s32)D_801F558F[record] << 16) >= second) {
      return i;
    }
  }
  return 0xFF;
}
