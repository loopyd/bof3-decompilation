#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is the named cell
 * symbol the exact bmagic siblings of this tree use: the symbolic address folds
 * into each load (lui $vN,0x1F80 / lw $vN,68($vN)), and the original
 * re-materialises the cell before every store group because the stores through
 * the previously loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared no-argument helper of the main executable. The sibling plchar lifts
 * declare the same prototype and keep only its returned byte as a ready test;
 * this body discards that byte, because the constant load after the call
 * overwrites $v0. This target's generated undefined_funcs_auto.txt already binds
 * the address, so no target-map row is needed. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* @source 0x801EF7F4
 * @behavior Per-frame work-handler step of this overlay, the ninth handler of the
 *           ten-word code pointer run at 0x801EF8BC whose word 8 (0x801EF8DC)
 *           holds this address, reached through the state-byte-1 dispatcher
 *           dispatchStateByte1Tablef8bcBmagicMagic11503_801EF054, which indexes
 *           that run with the work object's byte 1 scaled by four. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and decrements the
 *           object's unsigned tint-channel bytes 0x5D, 0x5E and 0x5F by four in
 *           place (lbu / addiu -4 / sb), the same three channels the exact
 *           siblings decrementWorkColorTripletBy16...BmagicMagic05603_801F00C8
 *           and runSharedUpdateThenDecrementWorkByte9...BmagicMagic05603_801F042C
 *           fade and clear, so the triplet steps down towards its minimum. Each
 *           of the three stores re-materialises the cell first, because a store
 *           through the loaded pointer may alias it. It then runs the shared
 *           main-executable helper func_8014DAEC, whose returned byte it discards
 *           (the call's delay slot carries the byte-0x5F store, and the constant
 *           load after it overwrites $v0), re-reads the cell and writes -0x80
 *           into tint channel 0x5D through the signed byte view, because the
 *           original materialises the immediate as li $v0,-128 (addiu
 *           $v0,$zero,-0x80) rather than the 128 an unsigned char store would
 *           fold to, and re-reads the cell once more to advance
 *           work byte 2 by one (lbu / addiu 1 / sb), which moves the
 *           dispatcher's byte-2 step run forward. The 0x18-byte frame only keeps
 *           the return address at 0x10 across that single call; the step takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           0x94-byte span is not byte identical to any of the 396 payloads under
 *           out/binaries: a full-payload search for these 148 bytes returns only
 *           this target's own copy at offset 0xBF4, so no clone seed exists and
 *           the statement order above comes from the byte-identical shape
 *           evidence of the named siblings.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkTintTripletByFourThenInvokeSharedHelperSetChannel5DToMinus80AndAdvanceWorkByte2BmagicMagic11503_801EF7F4(void) {
  g_battle_work[0x5D] -= 4;
  g_battle_work[0x5E] -= 4;
  g_battle_work[0x5F] -= 4;
  func_8014DAEC();
  ((s8 *)g_battle_work)[0x5D] = -0x80;
  g_battle_work[2]++;
}
