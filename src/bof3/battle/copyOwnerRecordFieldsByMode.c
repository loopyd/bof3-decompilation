#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC870
 * @behavior Copies one field group of a caller-supplied source record into the
 * battle record selected by the owner id, choosing the group by mode: mode 0
 * copies the byte at +0x4B, the words at +0x50 and +0x54, the halfwords at
 * +0x58 and +0x5A and raises bit 0x20 of the record byte at +0x07; mode 1
 * copies the word triple at +0x34, +0x38 and +0x3C; every other mode copies
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the two mode groups are a switch with no default arm, so the
 * dispatch tests sit inline and both groups are emitted out of line with the
 * no-mode path jumping to the epilogue. The u8 owner id is zero-extended in the
 * findOwnerRecord `jal` delay slot and the returned record pointer stays in
 * $a0, so every copy is a direct load/store pair with no address temporary.
 * The mode-0 flag raise is written after both halfword copies, which is the
 * order that lets the scheduler read byte +0x07 ahead of the +0x5A load and
 * leave the +0x5A store in the `j` delay slot; raising the flag between the two
 * halfword copies instead measures 38/49 instructions and 192 bytes.
 */
void copyOwnerRecordFieldsByMode(u8 id, u8 *source, u8 mode)
{
  u8 *record;

  record = findOwnerRecord(id);
  switch (mode) {
  case 0:
    record[0x4B] = source[0x4B];
    *(u32 *)(record + 0x50) = *(u32 *)(source + 0x50);
    *(u32 *)(record + 0x54) = *(u32 *)(source + 0x54);
    *(u16 *)(record + 0x58) = *(u16 *)(source + 0x58);
    *(u16 *)(record + 0x5A) = *(u16 *)(source + 0x5A);
    record[0x07] |= 0x20;
    break;
  case 1:
    *(u32 *)(record + 0x34) = *(u32 *)(source + 0x34);
    *(u32 *)(record + 0x38) = *(u32 *)(source + 0x38);
    *(u32 *)(record + 0x3C) = *(u32 *)(source + 0x3C);
    break;
  }
}
