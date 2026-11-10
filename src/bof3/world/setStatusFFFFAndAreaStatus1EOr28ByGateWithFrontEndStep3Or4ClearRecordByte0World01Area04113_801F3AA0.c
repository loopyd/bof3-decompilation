#include "bof3/bof3.h"

/* Shared gate byte this overlay's entry handlers branch on. It is read with lb,
 * so it is declared signed here.
 * @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;

/* Shared status halfword the entry handlers latch, stored with sh.
 * @source 0x801490A8 @kind unknown */
extern u16 D_801490A8;

/* Shared area status byte the entry handlers seed, stored with one sb.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Pointer cell publishing the active work record. The original reads the cell
 * with one lw and stores through the pointer, so the pointee is a plain u8.
 * @source 0x80148208 @kind unknown */
extern u8 *D_80148208;

/* Shared front-end step helper at 0x801BE1B0, selected by a step number. */
void func_801BE1B0(u32 step);

/* @source 0x801F3AA0
 * @behavior Overlay entry handler word at 0x801F55FC, the second word of the
 *           pair that starts at 0x801F55F8 in the local handler pointer run
 *           based at 0x801F42C8. Latches 0xFFFF into the shared status
 *           halfword 0x801490A8 (`ori v0,0,0xFFFF`, `sh`), then branches on
 *           the shared gate byte 0x801490C7 read with `lb`. While that byte
 *           reads zero it runs the shared front-end step helper 0x801BE1B0
 *           with step 3 and yields 0x1E; once the gate byte is set it runs the
 *           same helper with step 4, stores zero into the byte the work-record
 *           pointer cell 0x80148208 addresses and yields 0x28. Both arms
 *           converge on one `sb` of that value to the shared area status byte
 *           0x80146865. Takes no arguments, returns nothing and reserves a
 *           0x18-byte frame for its two calls.
 *
 * Name evidence: the two arms differ only in the front-end step they pass, the
 *   area status byte value they leave behind and the work-record byte only the
 *   gate-set arm clears, so the name carries all three. The neighbour at
 *   0x801F3A48 (setStatusFFFFAndAreaStatus0AOr14ByGateWithFrontEndStep1Or2...)
 *   is the same latch-then-branch shape with steps 1/2 and 0x0A/0x14, and this
 *   word follows it in the same handler pointer run. 0x80148208 is the shared
 *   active work-record pointer cell: resetWorkRecordAndPublishRecordIndexThenState4Substate3World01Area06813_801F3F48
 *   declares it as its own record pointer and other overlays store single bytes
 *   through it. The suffix carries the in-payload address because no
 *   caller-visible role is evidenced beyond that table word.
 *
 * Shape note: the original carries the record pointer and the converged status
 *   value in one register, `$v0`, which the `lw` at 0x801F3AE0 and the `addiu`
 *   at 0x801F3AEC show back to back around the `sb` at 0x801F3AE8. One local
 *   therefore holds both: the pointer as its address, then the status constant.
 *   Splitting that into a pointer local plus a status local measured 21 of 26
 *   instructions with the status value demoted to `$v1` and a 104 to 100 byte
 *   deficit, because this compiler does not give two non-overlapping locals the
 *   same register. Both arms still write their own step number and constant and
 *   the gate-set arm alone stores the zero byte, as the original bytes show.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndAreaStatus1EOr28ByGateWithFrontEndStep3Or4ClearRecordByte0World01Area04113_801F3AA0(
    void) {
  u32 areaStatus;

  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_801BE1B0(3);
    areaStatus = 0x1E;
  } else {
    func_801BE1B0(4);
    areaStatus = (u32)D_80148208;
    *(u8 *)areaStatus = 0;
    areaStatus = 0x28;
  }
  D_80146865 = areaStatus;
}
