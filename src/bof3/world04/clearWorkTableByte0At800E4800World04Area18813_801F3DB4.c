#include "bof3/bof3.h"

/* @source 0x801F3DB4
 * @behavior Overlay work-table clear: zeroes byte 0 of the first 8 records of
 * the 0x18-byte-stride table at 0x800E4800 (the main-RAM work area this
 * overlay shares with the world and scenario overlays; the same base is cleared
 * with a 0x2C stride by clearWorkTableByte3At800E4800World04Area17413_801F2E5C
 * and with a 0x28 stride in emi/scenario). This target owns its own 8-entry,
 * 0x18-byte-stride, byte-0 boundary and its unsigned byte counter, which the
 * loop masks to 0xFF before comparing below 8, so entries 0 through 7 are
 * cleared. The record pointer advances by 0x18 per iteration and is scheduled
 * into the back edge's delay slot; the function takes no arguments, makes no
 * call and returns nothing.
 *
 * Evidence: payload asm at 0x801F3DB4 -- lui/ori of 0x800E4800 into $a0,
 * $v1 = 0, body sb $zero,0x0($a0) / addiu $v1,$v1,0x1 / andi $v0,$v1,0xFF /
 * sltiu $v0,$v0,8 / bnez + addiu $a0,$a0,0x18 in the delay slot, then jr $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableByte0At800E4800World04Area18813_801F3DB4(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x18u;
    i += 1u;
  } while (i < 8u);
}
