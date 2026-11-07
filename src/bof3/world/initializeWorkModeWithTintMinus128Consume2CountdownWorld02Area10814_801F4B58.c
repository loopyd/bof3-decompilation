#include "bof3/bof3.h"

/* The overlay's work record is published through the scratchpad pointer cell at
 * 0x1F800044, bound as a named target-local symbol rather than through the
 * constant-address accessor because this function re-reads the cell in five
 * separate groups: one base per group is what re-materialises
 * `lui $vN,0x1F80` plus the folded `lw $vN,68($vN)` the original bytes show. The
 * cell is a plain pointer: the stores through the loaded record already
 * separate the reads, so no volatility is needed. */
extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */

/* Global work record published by the pointer cell at 0x80146884. The function
 * stores one effect byte at +0x93 and reads and writes back the countdown
 * halfword at +0x7E through one shared base. */
extern u8* D_80146884; /* @source 0x80146884 @kind data */

/* Shared EXE helper at 0x8019651C, outside this overlay's payload and still
 * unlifted, so only its prototype is declared. It takes the work record plus
 * three zero offsets and the mode word 1 and returns the submitted effect byte
 * this function stores. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @source 0x801F4B58
 * @behavior Entry 0 of the work-byte-4 handler table at 0x801F63C4, dispatched
 *           by the byte-4 selector step at 0x801F4B14. It boots the work record
 *           published at the scratchpad pointer cell 0x1F800044: submits the
 *           record through the shared helper 0x8019651C with three zero offsets
 *           and mode 1, stores the returned effect byte at +0x93 of the record
 *           published by the pointer cell 0x80146884, sets the record's flag bit
 *           0x20, clears its flag bit 0x40, writes 1 into its 0x5C byte and 1
 *           into its handler byte +0x04, writes -0x80 into the tint triplet at
 *           +0x5D, +0x5E and +0x5F through one pointer load, and finally
 *           consumes two units of the 0x80146884 countdown halfword at +0x7E
 *           through its own base. Every access group re-reads both cells, so six
 *           `lui`/`lw` pairs appear; the 0x20-byte frame holds only $ra and the
 *           single call keeps its `nop` delay slot. Byte-identical to the exact
 *           sibling at 0x801ACC08 of emi/etc/game/00 apart from that -2 countdown
 *           immediate, which the sibling writes as -1 there. Takes no arguments
 *           and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkModeWithTintMinus128Consume2CountdownWorld02Area10814_801F4B58(void) {
  u8* record;
  u8* work;

  D_80146884[0x93] = func_8019651C(workRecord44World02Area10814, 0, 0, 0, 1);
  workRecord44World02Area10814[0] |= 0x20;
  workRecord44World02Area10814[0] &= 0xbf;
  workRecord44World02Area10814[0x5c] = 1;
  record = workRecord44World02Area10814;
  *(s8*)(record + 0x5f) = -0x80;
  *(s8*)(record + 0x5e) = -0x80;
  *(s8*)(record + 0x5d) = -0x80;
  workRecord44World02Area10814[4] = 1;
  work = D_80146884;
  *(u16*)(work + 0x7e) -= 2;
}
