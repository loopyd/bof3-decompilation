#include "bof3/bof3.h"
#include "bof3/battle/battle03_internal.h"

/* @source 0x801DD26C
 * @behavior decrements the queued battle index and writes one signed byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void storeQueuedIndexByte(s8 arg0) {
  volatile u8* index_ptr;
  u8 index;

  index_ptr = &BATTLE_GLOBAL_BYTE_6322;
  index = *index_ptr - 1;
  *index_ptr = index;
  (&D_8014630C)[index] = arg0;
}
