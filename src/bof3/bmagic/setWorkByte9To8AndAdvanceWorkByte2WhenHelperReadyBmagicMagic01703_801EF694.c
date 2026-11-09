#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named fixed address so
 * every access re-materializes the cell with its own lui plus offset load,
 * which is how the original reaches it once per store group. The exact
 * sibling setWorkByte9To15AndAdvanceWorkByte2WhenHelperReadyBmagicMagic13103
 * _801EFEF0 reads the same cell the same way. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared readiness helper of the main executable, outside this overlay's
 * image; the sibling bmagic, battle and plchar lifts declare the same
 * prototype. The original keeps only the returned byte as the ready test, so
 * the declared return type is that byte. Splat binds the address through this
 * target's generated undefined_funcs_auto.txt, so no target-map row is needed. */
extern u8 func_8014DAEC(void);

/* @source 0x801EF694
 * @behavior Step handler of this overlay's handler run: the word at
 *           0x801EEC24 holds this function's address, index 5 of the ten words
 *           the run dispatcher func_801EF058 copies from 0x801EEC10 into its
 *           frame and indexes with scratchpad work-object byte 2, so the body
 *           runs while work byte 2 is 5. That dispatcher is itself the single
 *           entry of the run at 0x801EF990 which the work-byte-1 dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic01703_801EF014 selects, and no
 *           jal in this payload names this function, so the table word is its
 *           only incoming path. Per call it runs the shared readiness helper
 *           func_8014DAEC and tests the returned byte zero-extended (andi 0xff,
 *           beqz); the body writes nothing while the helper reports not ready.
 *           Once it reports ready the handler re-materializes the work-object
 *           pointer cell at 0x1F800044 and stores 8 into the work object's byte
 *           9, then reads that cell again and increments the work object's byte
 *           2 with a byte-width add (lbu / addiu 1 / sb), advancing the run to
 *           index 6 at 0x801EF6EC. The 8 constant is materialized ahead of the
 *           guard branch in its delay slot. The 0x18-byte frame only keeps $ra
 *           at 0x10 across the helper call; the function takes no arguments,
 *           returns nothing, and reads or writes no other state. Roles of work
 *           byte 9 and of the shared helper are not established here, so the
 *           name describes the effect. Bytes 0xA94..0xAEB are identical to the
 *           complete exact sibling
 *           setWorkByte9To15AndAdvanceWorkByte2WhenHelperReadyBmagicMagic13103
 *           _801EFEF0 at 0x12F0 of its own payload in all twenty-one words
 *           except the one at +0x18, which stores this function's own 8 where
 *           that sibling stores 0xF; the statement order and operand forms come
 *           from that sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagicMagic01703_801EF694(void) {
  if (func_8014DAEC() != 0u) {
    D_1F800044[9] = 8;
    D_1F800044[2]++;
  }
}
