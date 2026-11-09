#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x80146254 @kind unknown — live count of the local work records. */
extern u8 D_80146254;
/* @source 0x80144908 @kind unknown — party mirror of the live local work
 * records, written slot by slot by
 * copyRecordPartyBytesThenInvokeHelperScenarioScena0900_801FE234. */
extern u8 D_80144908[];

/* @source 0x801FDF54
 * @behavior Overlay handler reached only indirectly: the address is word 23
 * (the word stored 0x801FE518) of the 38-entry in-image code-pointer table at
 * 0x801FE4BC that func_801FD280 (0x801FD280) indexes with the signed byte
 * returned by func_801A7AD8(0x801FE474, 0x0E) and calls with jalr, so no
 * in-image jal targets this address. It clears the first three slots of the
 * D_80144908 party mirror while remembering the last slot byte whose value is
 * neither 7 nor 2 - those two bytes are cleared but never selected - and it
 * keeps the initial 8 when every visited slot holds 7 or 2. With that id it
 * re-arms the overlay effect group through func_80166E88(7, 2, id, 0), where
 * a0=7, a1=2, a2 is the selected id and a3 is zeroed in that call's delay
 * slot, then clears the shared live-local-work-record count byte D_80146254,
 * runs the per-id teardown func_801C187C for the ids 7, 2 and the selected id
 * in that order, and finally calls func_801C1630 with no arguments. It takes
 * no arguments, reads no other memory and returns nothing; the 0x18-byte frame
 * only keeps $ra and $s0 across the calls, $s0 holds the selected id while $a0
 * carries the loop index, the loop bounds the zero-extended index with an
 * unsigned compare against 3 and the comparison constants 7 and 2 stay in $a2
 * and $a1 across the whole loop.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearPartySlotsThenRearmEffectGroupAndResetEffectSlotsScenarioScena0900_801FDF54(
    void) {
  u8 slot;
  u8 id;
  u8 i;

  i = 0;
  id = 8;
  for (; i < 3; i++) {
    slot = D_80144908[i];
    if (slot != 7 && slot != 2) {
      id = slot;
    }
    D_80144908[i] = 0;
  }
  func_80166E88(7, 2, id, 0);
  D_80146254 = 0;
  func_801C187C(7);
  func_801C187C(2);
  func_801C187C(id);
  func_801C1630();
}
