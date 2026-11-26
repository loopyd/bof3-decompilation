#include "bof3/bof3.h"

/* Word +0x34 of the 0x140-byte-stride local work records based at 0x80145E90:
 * the first word of the record's position pair, addressed here through a byte
 * offset so both words keep their own symbol base. The caller of this query
 * computes that record base as D_80145E90 + index * 0x140 and publishes it at
 * 0x1F800044 before asking for the index.
 * @source 0x80145EC4 @kind unknown
 */
extern u8 D_80145EC4[];
/* Word +0x38 of the same work records: the second word of the position pair.
 * @source 0x80145EC8 @kind unknown
 */
extern u8 D_80145EC8[];

/* Five-byte bounds records of the table at 0x801F3B9C. Byte +0 and byte +2
 * bound the +0x34 word, byte +1 and byte +3 bound the +0x38 word, and byte +4
 * is not read here: it is the record's kind byte, which the caller reads as
 * D_801F3BA0[index * 5] and hands to func_8014D8D4. Each bound byte is the
 * integer part of a 16.16 value, so it is compared after a 16-bit shift.
 * @source 0x801F3B9C @kind table
 */
extern u8 D_801F3B9C[];
extern u8 D_801F3B9D[];
extern u8 D_801F3B9E[];
extern u8 D_801F3B9F[];

/* @source 0x801F355C
 * @behavior Overlay query: reads the two position words (+0x34 and +0x38) of the
 *           0x140-byte-stride local work record selected by the byte argument,
 *           then walks the two five-byte bounds records of the table at
 *           0x801F3B9C and returns the index of the first record whose byte
 *           bounds contain both words, or 0xFF when none does. Takes one byte
 *           argument, uses no stack frame and makes no calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findWorkPositionBounds(u8 index) {
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
  for (; i < 2; i++, record += 5) {
    if (first >= ((s32)D_801F3B9C[record] << 16) &&
        ((s32)D_801F3B9E[record] << 16) >= first &&
        second >= ((s32)D_801F3B9D[record] << 16) &&
        ((s32)D_801F3B9F[record] << 16) >= second) {
      return i;
    }
  }
  return 0xFF;
}
