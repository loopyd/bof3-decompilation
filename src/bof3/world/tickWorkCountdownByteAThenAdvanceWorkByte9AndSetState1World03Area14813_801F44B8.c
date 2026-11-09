#include "bof3/bof3.h"

/* Scratchpad pointer cell holding this overlay's work record. The cell is
 * reloaded for every access, which is what the original bytes show. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Record builder of this overlay: the routine at 0x801F4798 takes the record
 * addresses +0x34 and +0x18. It is still an asm subsegment of this target, so
 * only a prototype is needed here. */
void func_801F4798(s32 arg0, s32 arg1);

/* Record step of this overlay: the routine at 0x801F4614 runs the record
 * table update. It is a later code span of the same overlay and carries its
 * own target-map row. */
void updateRecordTableFromProbeOffsetsAndStepFoundRecordOctantWorld03Area14813_801F4614(
    void);

/* @source 0x801F44B8
 * @behavior Record step of this overlay. Loads the work record pointer from the
 *           scratchpad cell 0x1F800044, passes the record addresses +0x34 and
 *           +0x18 to the record builder at 0x801F4798 and then runs the record
 *           table update at 0x801F4614. Decrements the record countdown byte at
 *           +0xA and, when that byte reaches zero, increments the record byte
 *           at +9 and writes 1 into the record byte at +1: the byte the
 *           state-byte-1 dispatcher of this overlay, whose handler table is
 *           based at 0x801F6160, indexes by four. Takes no arguments, returns
 *           nothing and keeps a 0x18-byte frame whose only saved register is ra
 *           at +0x10; the scratchpad record pointer cell is reloaded for each
 *           access and is loaded before the frame is created.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickWorkCountdownByteAThenAdvanceWorkByte9AndSetState1World03Area14813_801F44B8(
    void) {
  func_801F4798((s32)(D_1F800044 + 0x34), (s32)(D_1F800044 + 0x18));
  updateRecordTableFromProbeOffsetsAndStepFoundRecordOctantWorld03Area14813_801F4614();
  D_1F800044[0xA]--;
  if (D_1F800044[0xA] == 0) {
    D_1F800044[9]++;
    D_1F800044[1] = 1;
  }
}
