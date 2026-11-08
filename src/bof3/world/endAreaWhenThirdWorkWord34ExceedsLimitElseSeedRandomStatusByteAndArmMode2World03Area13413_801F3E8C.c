#include "bof3/bof3.h"

extern int rand(void);
void func_80196070(void);

/* Third shared work base of this overlay: the 0x140-byte-stride group the
 * overlay's third-base seeders publish as the scratchpad work cursor. */
extern u8 D_80146110[]; /* @source 0x80146110 @kind data */
/* Word at +0x34 of that base, the limit this handler tests. */
extern s32 D_80146144; /* @source 0x80146144 @kind unknown */
/* Published work pointer beside the scratchpad work cursor. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
/* Scratchpad work-pointer cell; the original re-reads it for the mode-2 store
 * after restoring it, so the cell keeps its volatile form. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_80146865; /* @source 0x80146865 @kind unknown */
extern u8 D_80146866; /* @source 0x80146866 @kind unknown */
extern u8 D_80146867; /* @source 0x80146867 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */
/* Head of this payload's 0x801F5724 tail byte blob, the 16-entry table this
 * handler indexes with the low nibble of rand(); its row holds only 3, 4 and 5. */
extern u8 D_801F5724[]; /* @source 0x801F5724 @kind table */

/* @source 0x801F3E8C
 * @behavior Overlay step handler, entry 1 of the five-entry handler run based
 *           at 0x801F5804, which this target dispatches with the work record
 *           state byte at +1. When the shared area status byte D_80146865
 *           already reads 0xFE the body first resets the work-area header
 *           through func_80196070. It then publishes the third shared work base
 *           D_80146110 as the shared work pointer D_80146250 and as the
 *           scratchpad work cursor at 0x1F800044, keeping the caller cursor
 *           aside in a register. When the word at +0x34 of that base
 *           (D_80146144) reads above 0x33FFFF as a signed word the area ends:
 *           the status byte takes the 0xFF end marker, the shared bytes
 *           D_80146866 and D_80146867 are cleared, the shared sub-state byte
 *           D_80146875 takes 0x14, the caller cursor is restored and the
 *           header of that restored record is reset through func_80196070
 *           before returning. Otherwise it restores the caller cursor, seeds
 *           the shared status byte with the byte the 16-entry table at
 *           0x801F5724 holds at the low nibble of rand(), and then arms mode 2
 *           by storing 2 into the restored record's byte at +1 through a fresh
 *           read of the cell rather than through the held local. No jal in the
 *           payload reaches this address; its only in-image word is 0x801F5808,
 *           entry 1 of that handler run. Takes no arguments and returns
 *           nothing; the 0x18-byte frame keeps the status byte address in $s0
 *           at 0x10 and $ra at 0x14 across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name and shape evidence: the first 0x20 instructions of this body are
 * instruction-identical to the exact same-target sibling
 * endAreaWhenThirdWorkWord34ExceedsLimitElseStepWorkByte9AndArmMode1World03Area13413_801F40C8,
 * which fixes the statement order, the status-pointer local in $s0, the
 * volatile scratchpad cell and the signed comparison against 0x33FFFF; the two
 * members differ only in their else branch. That sibling is entry 3 of the same
 * run, and it names the same end-area path, so the end-area branch is shared
 * family behavior while the random status seed and the mode-2 store are this
 * member. Entry 0 at 0x801F3E50 arms mode 1 through the same restored record,
 * which is the family this mode-2 store belongs to.
 */
void endAreaWhenThirdWorkWord34ExceedsLimitElseSeedRandomStatusByteAndArmMode2World03Area13413_801F3E8C(
    void) {
  u8* work;
  u8* status;

  status = &D_80146865;
  if (*status == 0xFE) {
    func_80196070();
  }
  work = D_1F800044;
  D_80146250 = D_80146110;
  D_1F800044 = D_80146110;
  if (D_80146144 > 0x33FFFF) {
    *status = 0xFF;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0x14;
    D_1F800044 = work;
    func_80196070();
    return;
  }
  D_1F800044 = work;
  *status = D_801F5724[rand() & 0xF];
  D_1F800044[1] = 2;
}
