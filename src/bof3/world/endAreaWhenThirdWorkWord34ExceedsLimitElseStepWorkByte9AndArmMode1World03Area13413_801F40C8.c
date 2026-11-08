#include "bof3/bof3.h"

/* Third shared work base of this overlay: the 0x140-byte-stride group the
 * overlay's third-base seeders publish as the scratchpad work cursor. */
extern u8 D_80146110[];
/* Word at +0x34 of that base, the limit this handler tests. */
extern s32 D_80146144;
/* Published work pointer beside the scratchpad work cursor. */
extern u8* D_80146250;
/* Scratchpad work-pointer cell; the original re-reads it for the mode-1 store
 * after restoring it, so the cell keeps its volatile form. */
extern u8* volatile D_1F800044;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146875;
extern u16 D_80146876;

void func_80196070(void);

/* @source 0x801F40C8
 * @behavior State-byte-1 handler 3 of the five-entry local handler run at
 * 0x801F5804, the table the exact dispatcher
 * dispatchStateByte1Table5804World03Area13413_801F3E0C indexes with the work
 * record's byte at +1: when the area status byte D_80146865 already reads 0xFE
 * it first resets the work-area header through func_80196070, then publishes
 * the third shared work base D_80146110 as the overlay work pointer D_80146250
 * and as the scratchpad work cursor 0x1F800044 while keeping the caller's
 * cursor pointer aside in a register. When the word at +0x34 of that base
 * (D_80146144) reads above 0x33FFFF as a signed word, the area ends: the area
 * status byte takes the 0xFF end marker, the shared mode byte D_80146866 and
 * the shared byte D_80146867 are cleared, the shared sub-state byte D_80146875
 * takes 0x14, the caller's cursor is restored and the work-area header of that
 * restored record is reset through func_80196070 before returning. Otherwise it
 * restores the caller's cursor, reads that record's byte at +9 into a local and
 * stores the read value plus 0xFF back (the original spells the countdown step
 * as an addition of 0xFF rather than a subtraction), and when the read value
 * was zero it clears the shared halfword countdown D_80146876 and stores 1 into
 * the record's state byte at +1, i.e. arms the mode 1 that the exact sibling
 * setScratchStateToMode1WhenPadMatchesWorld03Area13413_801F3E50 also arms. That
 * mode-1 store goes through the cell again, so it re-reads the restored pointer
 * instead of reusing the local. No jal in the payload reaches this address; its
 * only in-image word is 0x801F5810, entry 3 of that handler run. Takes no
 * arguments and returns nothing; the 0x18-byte frame keeps the status-byte
 * address in $s0 at 0x10 across the calls and $ra at 0x14.
 * @status exact
 * @match 100.00
 * @residual none
 */
void endAreaWhenThirdWorkWord34ExceedsLimitElseStepWorkByte9AndArmMode1World03Area13413_801F40C8(
    void) {
  u8* work;
  u8* status;
  u8 count;

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
  count = work[9];
  work[9] = count + 0xFF;
  if (count == 0) {
    D_80146876 = 0;
    D_1F800044[1] = 1;
  }
}
