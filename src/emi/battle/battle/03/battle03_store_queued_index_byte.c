#include "bof3/bof3.h"
#include "internal.h"

/* @source 0x801DD26C
 * @behavior decrements the queued battle index and writes one signed byte.
 */
void battle03_store_queued_index_byte(s8 arg0) {
  volatile u8* index_ptr;
  u8 index;

  index_ptr = &BATTLE_GLOBAL_BYTE_6322;
  index = *index_ptr - 1;
  *index_ptr = index;
  (&D_8014630C)[index] = arg0;
}
