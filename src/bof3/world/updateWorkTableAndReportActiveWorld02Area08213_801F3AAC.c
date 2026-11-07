#include "bof3/bof3.h"

void func_801F387C(u8* record);

/* @source 0x801F3AAC
 * @behavior Overlay per-frame helper: walks the 0x80 entries of the
 *           0x14-byte-stride work table at 0x800E4800 (the main-RAM work area
 *           this overlay shares with the world and scenario overlays). For each
 *           entry whose lead byte is non-zero it subtracts the signed halfword
 *           at +0x02 shifted left by eight from that entry's +0x0C word and
 *           stores the difference back into the word, calls the per-record draw
 *           helper at 0x801F387C with the entry pointer, and, when the updated
 *           +0x0C word is below the +0x3C word of the scratch work record
 *           published through 0x1F800044, clears the entry lead byte; either way
 *           it marks the frame active. It returns the word 1 when at least one
 *           entry was active this frame, whether or not that entry was retired,
 *           and 0 when none was. It takes no arguments and uses a 0x20-byte
 *           frame holding s0/s1/s2 and the saved return address. The 176 bytes
 *           are a near-identical twin of the already-exact
 *           updateWorkTableAndReportActiveScenarioScena0100_801F7C54 lift in
 *           emi/scenario/scena01/00: the only differing word is that function's
 *           own call immediate (0x801F387C here against 0x801F7A24 there), which
 *           is the naming evidence for this body. Its only callers inside this
 *           payload are 0x801F3408 and 0x801F3470, whose `andi $v0,$v0,0xFF`
 *           proves low-byte consumption by the reporting call site.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 updateWorkTableAndReportActiveWorld02Area08213_801F3AAC(void) {
  s32 active;
  u8* record;
  u8 i;
  s32* value;

  active = 0;
  record = PSX_PTR(u8, 0x800E4800u);
  i = 0;
  value = PSX_PTR(s32, 0x800E480Cu);
  do {
    if (*record != 0) {
      *value -= FIELD_REF(s16, value, -10) * 256;
      func_801F387C(record);
      if (*value < FIELD_REF(s32, SPAD_PTR_SLOT(u8, 0x44), 0x3C)) {
        *record = 0;
      }
      active = 1;
    }
    value += 5;
    i += 1;
    record += 0x14;
  } while (i < 0x80);
  return active;
}
