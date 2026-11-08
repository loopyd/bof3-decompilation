#include "bof3/bof3.h"

/* The shared battle flag halfword whose bit 0x2 requests the front-end handoff;
 * the sibling handler raiseFlagBit2ThenEnterFrontendMode4 (0x800C2B2C) of this
 * same payload raises the same bit before the same func_8014ECAC(4) call and is
 * lifted with this non-volatile u16 declaration. */
extern u16 D_801462E8;

/* Main-RAM count byte of the 0x140-byte-stride local work records, owned by the
 * shared map (config/targets/shared/symbols.txt), so this target adds no row.
 * This handler leaves it at 3, the party size the front-end record list shows. */
extern u8 D_80146254;

/* Second main-RAM id byte list of the visible slots, owned by the shared map, so
 * this target adds no row. swapSelectionIds and the shop panel sources declare
 * it as the parallel list of D_80144F5A. */
extern u8 D_80144F5D[];

/* The scenario-archive scratch copy of that slot-id list. func_801F9DF4 of
 * emi/scenario/scena06/00 (0x801F9DF4) saves the live list into 0x801FE4EC and
 * the count byte D_80146254 into 0x801FE4E4 on the way out of the front end;
 * these two cells are the count and the byte run of that copy, 8 bytes apart.
 * Both addresses sit in the main-RAM area the scenario overlays load into (base
 * 0x801F6C00), not in this payload, and the generated undefined-symbol list of
 * this target carries them, so no target-map row is added. */
extern u8 D_801FE4E4;  /* @source 0x801FE4E4 @kind unknown */
extern u8 D_801FE4EC[]; /* @source 0x801FE4EC @kind unknown */

/* @source 0x800C2680
 * @behavior Second of the three handlers this overlay's installer func_800C2184
 * (0x800C2184) publishes into the shared battle hook slots 0x801463A4,
 * 0x801463A8 and 0x801463AC, so the battle step reaches it through slot
 * 0x801463A8 and no in-payload `jal` targets it. It first raises bit 0x2 of the
 * shared battle flag halfword D_801462E8 and requests frontend mode 4 through
 * func_8014ECAC, the same two-step handoff its sibling handler
 * raiseFlagBit2ThenEnterFrontendMode4 (0x800C2B2C) performs. It then reads the
 * saved count byte 0x801FE4E4, publishes it as the work-record count D_80146254
 * only so that byte can bound the copy, restores that many bytes of the saved
 * slot-id list from 0x801FE4EC into the live list D_80144F5D, and finally stores
 * 3 in D_80146254, the party size the front-end record list expects. The copy is
 * a byte loop driven by a u8 counter that indexes both byte arrays, so the bound
 * byte is reloaded from D_80146254 on every iteration because the id-list store
 * may alias it; the count test that enters the loop is the forwarded value of
 * that load, not a reload. The counter is a u8 and not a u32: the same loop with
 * a u32 counter measured the same 36 instructions and bytes but reserved an
 * 8-byte larger frame (0x20 with $ra at 0x18 instead of 0x10), which is the only
 * difference between the two spellings. The function takes no arguments, keeps
 * the family's 0x18-byte frame
 * with $ra at 0x10 across the one call, touches $v0/$v1/$a0/$at only, and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void enterFrontendMode4RestoreSavedSlotIdsAndSetRecordCount3BossBoss02513_800C2680(
    void) {
  u16* flags;
  u8 i;

  flags = &D_801462E8;
  *flags |= 2;
  func_8014ECAC(4);
  D_80146254 = D_801FE4E4;
  for (i = 0; (u8)i < D_80146254; i++) {
    D_80144F5D[(u8)i] = D_801FE4EC[(u8)i];
  }
  D_80146254 = 3;
}
