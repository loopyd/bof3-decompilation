#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F5678[];
extern u32 D_8014686C;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015DF18(u16 arg0);
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801F3B48
 * @behavior Overlay status setter with one activation sequence. It indexes the
 *           two-entry halfword table at 0x801F5678 with the shared gate byte
 *           D_801490C7 (`lb`, `sll $v1,$v0,1`, `addu $at,$at,$v1`, `lhu`) and
 *           stores the selected halfword into the shared status halfword
 *           D_801490A8; the table holds 0x54 at index 0 and 0xFFFF at index 1,
 *           so a clear gate publishes status 0x54 and a set gate publishes
 *           0xFFFF. On the clear-gate arm only it then tests shared scenario
 *           flag 6 of the flag word D_8014686C through func_8015B5D4: while
 *           that flag still reads set the function returns, and once it reads
 *           clear (`bnez $v0` on the helper result) it sends sound cue 0x106
 *           through func_8015DF18, runs the availability gate
 *           func_801650B4(0, 0x28, 1, 0) with a discarded result, and sets
 *           flag 6 again through func_8015B580. Takes no arguments, returns
 *           nothing, and its 0x18-byte frame only keeps $ra across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the status halfword is not a pair of immediates but a real
 * table read - the original words F58..F64 are the index scaling `sll
 * $v1,$v0,1` plus `addu $at,$at,$v1` and one `lhu` off 0x801F5678 - and the
 * image bytes at that address are the two halfwords 0x0054 and 0xFFFF followed
 * by the next, pointer-shaped table, so the span is exactly the status pair
 * selected by the gate byte. That gate polarity matches the sibling
 * setStatus64AndModeIndex6IfGateElseFFFFWorld01Area04113_801F3D78 and
 * setStatusFFFFAndStateEntryWhenGateClearWorld01Area04113_801F3B08, which use
 * the same shared status halfword and gate byte, so the name follows their
 * setStatus<values>ByGate grammar and adds the table source. The three calls
 * on the clear-gate arm are the shared flag test func_8015B5D4 (read with
 * flag 6), the main-exe sound cue dispatcher func_8015DF18 (cue 0x106, the
 * same cue the plchar claim handlers send after their own
 * func_801650B4(0, N, 1, 0) availability gate accepts), the gate itself with
 * the constants 0, 0x28, 1, 0, and the flag setter func_8015B580 with the same
 * flag 6, so the tail of the name names those call roles rather than a
 * dispatch role the overlay evidence does not show. The range has its own
 * prologue at 0x801F3B50 and its own `jr $ra` at 0x801F3BC4, and the next
 * function begins at 0x801F3BCC, so it is a whole function and not data or a
 * shared epilogue. No pointer run or dispatch table in the overlay lists
 * 0x801F3B48, so no in-overlay caller names an extra role.
 */
void setStatus54OrFFFFFromGateTableThenWaitFlag6Cue106ClaimAndSetFlagWorld01Area04113_801F3B48(
    void) {
  s8 gate;

  gate = D_801490C7;
  D_801490A8 = D_801F5678[gate];
  if (gate == 0) {
    if (func_8015B5D4(D_8014686C, 6) == 0) {
      func_8015DF18(0x106);
      func_801650B4(0, 0x28, 1, 0);
      func_8015B580(D_8014686C, 6);
    }
  }
}
