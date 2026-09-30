#include "bof3/ui/game00_internal.h"

/*
 * @behavior Entry-0 front-end record gate. Returns 0 while either world flag
 *           word 0x80146258 / 0x8014625A carries one of its low three bits
 *           set. Otherwise it walks the live 0x140-byte-stride local work
 *           records after the first one, bounded by the record count byte
 *           0x80146254: a record whose leading byte at 0x80145E91 reads 2 must
 *           carry a byte at 0x80145E92 that appears in the three-byte
 *           accepted-type list 0x801CD1C8, otherwise 0 is returned; records
 *           that are skipped or accepted fall through to 1.
 * @source 0x801B55C0
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The scaled byte offset is load-bearing as a separate local: writing
 * `i * 0x140` inline in both record accesses lets gcc hoist two symbol base
 * addresses as pointer induction variables (live 29/49 insns, first=+0x20);
 * the local keeps the original `%hi(sym) + offset` addressing, and the
 * compared operand has to be the record type byte so gcc emits the original
 * `beq a1,v0` orientation (live 48/49, first=+0x88 otherwise). */
s32 func_801B55C0(void) {
  u8  count;
  u8  type;
  s32 i;
  s32 j;
  s32 offset;

  if (((D_80146258 | D_8014625A) & 7) != 0) {
    return 0;
  }
  count = D_80146254;
  for (i = 1; i < count; i++) {
    offset = i * 0x140;
    if (D_80145E91[offset] == 2) {
      type = D_80145E92[offset];
      for (j = 0; j < 3; j++) {
        if (type == D_801CD1C8[j]) {
          break;
        }
      }
      if (j == 3) {
        return 0;
      }
    }
  }
  return 1;
}
