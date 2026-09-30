#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 scratch-record publish step: when the shared phase byte
 * 0x80145E92 reads 1 it clears the work-record state byte at scratch offset 2
 * and returns; otherwise, once the shared halfword 0x8014930E has reached
 * 0x39, it sets that state byte to 3 and copies the shared template bytes
 * 0x80145EBA/0x80145ED9/0x80145EDA/0x80145EDB into scratch offsets
 * 0x2A/0x49/0x4A/0x4B, the shared words 0x80145EE0/0x80145EE4 into scratch
 * offsets 0x50/0x54 and the shared halfwords 0x80145EE8/0x80145EEA into
 * scratch offsets 0x58/0x5A, then calls func_8014D4E0 without arguments.
 * @source 0x801D11C0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D11C0(void) {
  u8* work;

  if (D_80145E92 == 1) {
    D_1F800044[2] = 0;
    return;
  }
  if (D_8014930E >= 0x39) {
    /* The fixed-address cursor view, not the named D_1F800044 symbol: it is the
     * only representation that reproduces the original's constant-in-delay-slot
     * store schedule (li v0,3 ahead of the folded cursor load), because a single
     * use folds to lui/lw %lo instead of CSE-ing a shared cursor base
     * (sibling func_801DBEDC records the same lever). */
    WORLD00_AREA030_SCRATCH_PTR[2] = 3;
  }
  D_1F800044[0x2A] = D_80145EBA;
  D_1F800044[0x49] = D_80145ED9;
  D_1F800044[0x4A] = D_80145EDA;
  D_1F800044[0x4B] = D_80145EDB;
  work = D_1F800044;
  *(s32*)(work + 0x50) = D_80145EE0;
  *(s32*)(work + 0x54) = D_80145EE4;
  *(u16*)(work + 0x58) = D_80145EE8;
  *(u16*)(work + 0x5A) = D_80145EEA;
  func_8014D4E0();
}
