#include "bof3/bof3.h"

extern void* D_80146884; /* @source 0x80146884 @kind data */
extern u8 D_80146874;    /* @source 0x80146874 @kind unknown */
extern u8 D_80146875;    /* @source 0x80146875 @kind unknown */

void func_8015C088(void);

/* @source 0x801F3130
 * @behavior Runs the shared front-end startup helper 0x8015C088, then clears
 * bit 0 of the byte at offset 0x74 of the work record published at 0x80146884
 * with an `andi 0xFE` read-modify-write, increments that record's unsigned
 * halfword at offset 0x7E in place, and then requests primary state 2 and
 * substate 0x19 through the shared bytes 0x80146874 and 0x80146875. The
 * pointer cell 0x80146884 is re-read after the byte store, so the increment
 * goes through the record published then rather than the one whose bit 0 was
 * just cleared. Takes no arguments and returns nothing; the 0x18-byte frame
 * keeps only $ra because the record pointer is never held across the call, and
 * each of the two shared-byte stores materialises the 0x8014 page in $at on its
 * own, with the stored constant 2 and 0x19 loaded into $v0 immediately before
 * its store, so the statement order is the byte order after the increment. The
 * first 0x44 bytes are the lifted neighbour 0x801F30DC's body byte for byte
 * (verified by `cmp` of the two spans), which supplies the statement order and
 * operand forms of the startup call, the masked byte store and the re-read
 * increment; this span continues past that point with the two shared-byte
 * stores and a `lw $ra` epilogue whose `jr` delay slot is a `nop`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runStartupAnimPrimary(
    void) {
  u8* record;

  func_8015C088();
  record = D_80146884;
  record[0x74] &= ~1u;
  record = D_80146884;
  *(u16*)(record + 0x7E) += 1;
  D_80146874 = 2;
  D_80146875 = 0x19;
}
