#include "bof3/bof3.h"

/* Work-area record pointer cell of the shared main executable, immediately
 * before the D_80146888 work-area array. It is outside this overlay's image, so
 * this overlay family's neighbouring lifts read the same cell.
 * @source 0x80146884 @kind data
 */
extern u8* D_80146884;

/* Byte +0x79 of the first 0x140-stride local work record of the shared run at
 * 0x80145E90. The neighbouring overlays read the same byte as that record's
 * party id and as its kind; this body only loads it, with `lbu`, to index the
 * table below, so it stays a byte.
 * @source 0x80145F09 @kind data
 */
extern u8 D_80145F09;

/* Party-id to field-record index table of the shared main executable. The
 * World04 area191/area192 lifts index the 0xA4-byte field record array
 * 0x80144968 with `D_80181B10[code]`, so this reads one entry of that same
 * mapping.
 * @source 0x80181B10 @kind table
 */
extern u8 D_80181B10[];

/* Flag bank of the shared main executable that the shared bit setter below
 * writes. This body passes its address, so it is declared as the object the
 * address names.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Shared bit setter of the main executable: writes the bit its second argument
 * selects in the flag bank its first argument addresses.
 * @source 0x8015B5F4
 */
extern void func_8015B5F4(void* flag_bank, u8 bit_index);

/* @source 0x801F4804
 * @behavior Clears bit 0 of the byte at +0x74 of the work record published by
 *           the shared pointer cell 0x80146884 with an `andi 0xFE`
 *           read-modify-write. When the shared party-id byte 0x80145F09 indexes
 *           table 0x80181B10 to the value 1, it sets bit 0xD of the shared
 *           main-RAM flag bank 0x80144F28 through the shared setter 0x8015B5F4;
 *           otherwise it only performs the read-modify-write. Takes no
 *           arguments and returns nothing. The 0x18-byte frame keeps only $ra
 *           at +0x10.
 *
 * Shape evidence: overlay World02 area108 target emi/world02/area108/14 holds
 *   func_801F4A98 at src/bof3/world/
 *   clearGlobalWorkFlagBit0AndResetScratchModeWhenMappedState5World02Area10814_801F4A98.c,
 *   an exact lift whose body is this one statement for statement: the record
 *   pointer cell load hoisted above the frame creation, the +0x74 read, the
 *   0xFE mask and the store back, then one gated block holding the shared flag
 *   bank call, then the single epilogue. Overlay World03 area140's lift of
 *   func_801F3DAC carries the same ordering with the same `~1u` mask spelling.
 *   Those sources supplied the statement order and the pointer-local spelling;
 *   every literal here is written from this span's own bytes: gate 1 rather
 *   than 5, flag bit 0xD rather than 0x30.
 *
 *   The caller is unnamed: no `jal` and no word of this payload holds this
 *   span's address, so the argument roles stay structural.
 *
 * Clone declined: a scan of all 396 extracted payloads under out/binaries for
 *   this 0x60-byte body, with its `jal` immediate word ignored so a
 *   relocation-different sibling could still match, returned only this span's
 *   own offset in its own payload. The family members each carry their own
 *   table base and bit constant, so no member can be byte-identical either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte74Bit0ThenSetSharedFlagBankBitDWhenMappedStateIs1World01Area04813_801F4804(
    void) {
  u8* record;

  record = D_80146884;
  record[0x74] &= ~1u;
  if (D_80181B10[D_80145F09] == 1) {
    func_8015B5F4(&D_80144F28, 0xD);
  }
}
