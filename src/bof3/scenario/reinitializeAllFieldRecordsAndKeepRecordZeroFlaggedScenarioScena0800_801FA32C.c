#include "bof3/bof3.h"

extern u8 D_8014496F[];

void func_801C16B8(void);

/* @source 0x801FA32C
 * @behavior Field-record reset run by this overlay's phase-1 entry handler
 * 0x801FA2D0: it sets bit 0 of the flag byte at 0x8014496F in all eight
 * 164-byte-stride field records of the table at 0x80144968, then calls the
 * shared field-record reinitializer func_801C16B8 in the etc/game overlay,
 * which only visits flagged records (it clears each one's byte 0x1A, calls
 * 0x80164A44 with the record base, zeroes the halfword at 0x0C and re-seeds
 * the halfwords at 0x14/0x16 and the byte at 0x18 from that record's
 * 0x1C/0x1E/0x2A cells), and finally clears bit 0 of records 1 through 7 so
 * record 0 is the only record left flagged. Both passes walk an unsigned byte
 * index over the fixed stride in a bottom-tested loop. It takes no arguments,
 * returns nothing and its 0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void reinitializeAllFieldRecordsAndKeepRecordZeroFlaggedScenarioScena0800_801FA32C(void) {
  /* Separate pass-local offsets keep each scale calculation block-local. */
  s32 setOffset;
  s32 clearOffset;
  u8 i;

  i = 0;
  do {
    setOffset = i * 0xA4;
    D_8014496F[setOffset] = D_8014496F[setOffset] | 1;
    i++;
  } while (i < 8);
  func_801C16B8();
  i = 1;
  do {
    clearOffset = i * 0xA4;
    D_8014496F[clearOffset] = D_8014496F[clearOffset] & 0xFE;
    i++;
  } while (i < 8);
}
