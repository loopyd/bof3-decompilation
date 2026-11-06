#include "bof3/bof3.h"

/* Word +0x34 of the 0x140-byte-stride local work records based at 0x80145E90:
 * the first word of the record's position pair, addressed here through a byte
 * offset so both words keep their own symbol base.
 * @source 0x80145EC4 @kind unknown
 */
extern u8 D_80145EC4[];
/* Word +0x38 of the same work records: the second word of the position pair.
 * @source 0x80145EC8 @kind unknown
 */
extern u8 D_80145EC8[];

/* Six-byte bounds records. Byte +0 and byte +2 bound the +0x34 word, byte +1
 * and byte +3 bound the +0x38 word, and bytes +4/+5 are the two ids the caller
 * publishes for the matching record. Each bound byte is the integer part of a
 * 16.16 value, so it is compared after a 16-bit shift.
 * @source 0x801F5994 @kind table
 */
extern u8 D_801F5994[];
extern u8 D_801F5995[];
extern u8 D_801F5996[];
extern u8 D_801F5997[];

/* @source 0x801F4C08
 * @behavior Overlay query: reads the two position words (+0x34 and +0x38) of the
 *           0x140-byte-stride local work record selected by the byte argument,
 *           then walks the six-byte bounds records of the table at 0x801F5994
 *           and returns the index of the first record whose byte bounds contain
 *           both words, or 0xFF when none does. Only the first record is ever
 *           visited: the counter is compared against zero, so the shipped loop
 *           runs once. Takes one byte argument, uses no stack frame and makes no
 *           calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findWorkPositionBoundsIndexWorld03Area11813_801F4C08(u8 index) {
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
    if (first >= ((s32)D_801F5994[record] << 16) &&
        ((s32)D_801F5996[record] << 16) >= first &&
        second >= ((s32)D_801F5995[record] << 16) &&
        ((s32)D_801F5997[record] << 16) >= second) {
      return i;
    }
  }
  return 0xFF;
}
