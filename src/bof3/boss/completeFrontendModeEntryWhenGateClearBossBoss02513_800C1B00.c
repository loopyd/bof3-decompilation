#include "bof3/bof3.h"

void func_800AC844(s32 arg0);

/* Shared battle flag halfword. Bit 0x2 is the pending frontend-mode entry the
 * exact sibling 0x800C2B2C raises before it requests frontend mode 4 through
 * func_8014ECAC. */
extern u16 D_801462E8;

/* Global gate halfword. The lifted UI and shop sources that test it read it as
 * a busy/in-progress gate and act only while it is clear. */
extern u16 D_80143C40; /* @source 0x80143C40 @kind unknown */

/* Shared effect-bank byte: the exact sibling 0x800C1CB4 of BIN/BOSS/BOSS007.EMI#16
 * clears it on the same battle-flag clear path. */
extern u8 D_8014832E; /* @source 0x8014832E @kind unknown */

/* @source 0x800C1B00
 * @behavior Single handler word of this overlay's published handler run. It
 *           loads the shared battle flag halfword 0x801462E8 once through one
 *           local pointer and returns immediately unless bit 0x2 of it is set;
 *           with that pending frontend-mode entry set it then reads the global
 *           gate halfword 0x80143C40 and returns while that gate is non-zero.
 *           Only with the request set and the gate clear does it act: it calls
 *           func_800AC844 with owner id 0, which strips the 0x40 flag of owner
 *           0's record in the shared work table, stores zero into the shared
 *           effect-bank byte 0x8014832E, and then clears bit 0x2 of 0x801462E8
 *           so the pending frontend-mode entry is retired. It takes no
 *           arguments, returns nothing and keeps $ra in a 0x18-byte frame.
 *           Bit 0x2 is the flag the exact sibling
 *           raiseFlagBit2ThenEnterFrontendMode4BossBoss02513_800C2B2C raises
 *           immediately before it requests frontend mode 4 through
 *           func_8014ECAC, so this body is the clear-side counterpart of that
 *           request: it waits for the gate halfword and then retires the flag
 *           together with the owner-0 0x40 flag and the effect bank. Its
 *           address appears in exactly one word of the payload, 0x800C31EC,
 *           the last word of the five-word group 0x800C31DC..0x800C31EC whose
 *           previous word is the exact noop handler
 *           noopHandlerBossBoss02513_800C1AF8; that word lies inside the record
 *           template blob the setup sibling func_800C1BB0 publishes into +0xEC
 *           of the overlay work record, and no `jal` or `jalr` in the payload
 *           reaches it, so the main program reaches it through that run rather
 *           than through a missing in-payload caller. The store order below is
 *           the byte order: the effect-bank store sits between the reload of
 *           0x801462E8 and the mask that clears its bit, so the flag is reloaded
 *           after the call and masked in the same statement that writes it back.
 * @status exact
 * @match 100.00
 * @residual none
 */
void completeFrontendModeEntryWhenGateClearBossBoss02513_800C1B00(void) {
  u16* flags;

  flags = &D_801462E8;
  if ((*flags & 0x2) != 0 && D_80143C40 == 0) {
    func_800AC844(0);
    D_8014832E = 0;
    *flags &= 0xFFFD;
  }
}
