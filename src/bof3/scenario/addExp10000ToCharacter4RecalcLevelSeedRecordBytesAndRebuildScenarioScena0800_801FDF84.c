#include "bof3/bof3.h"

/* Character record 4 of the shared 0xA4-byte-stride record table at 0x80144968,
 * seen from its experience counter: 0x80144C00 is that record's +0x08 word,
 * 0x80144BF8 (this base minus 8) its base, and 0x80144C06..0x80144C09 its bytes
 * at +0x0E..+0x11. func_801ADDD4 in the concurrently loaded etc/game overlay
 * reads the same +0x08 word as the record's experience total and recomputes the
 * level from it, so index 4 passed to all three helpers is this record's own
 * index. */
extern u8 D_80144C00[];
extern u8 D_80144C06;
extern u8 D_80144C07;
extern u8 D_80144C08;
extern u8 D_80144C09;

void func_801ADDD4(s32 character);
void func_80164A44(u8* record);
void func_80165280(s32 character, s32 arg1, s32 arg2);
void func_8016534C(s32 character, s32 arg1, s32 arg2);

/* @source 0x801FDF84
 * @behavior Awards 10000 experience to character record 4: the base
 * 0x80144C00 is materialised once and serves the load and the store of that
 * record's +0x08 experience word and then, eight bytes earlier, the record
 * base handed to the field-record rebuild helper func_80164A44. After the
 * award it runs the level recalculation func_801ADDD4 with the record index 4,
 * writes 7, 0x38, 0x21 and 0x0E into the record bytes at 0x80144C06 through
 * 0x80144C09 - each store re-materialises its own symbol base - rebuilds the
 * record through func_80164A44, and finishes with two per-character helpers
 * func_80165280 and func_8016534C, both taking the record index 4 with two
 * zero arguments. Takes no arguments and returns nothing; the 0x18-byte frame
 * keeps $ra plus $s0 for that shared record base across the first call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addExp10000ToCharacter4RecalcLevelSeedRecordBytesAndRebuildScenarioScena0800_801FDF84(void) {
  u8* recordExp;

  recordExp = D_80144C00;
  *(u32*)recordExp += 10000;
  func_801ADDD4(4);
  D_80144C06 = 7;
  D_80144C07 = 0x38;
  D_80144C08 = 0x21;
  D_80144C09 = 0x0E;
  func_80164A44(recordExp - 8);
  func_80165280(4, 0, 0);
  func_8016534C(4, 0, 0);
}
