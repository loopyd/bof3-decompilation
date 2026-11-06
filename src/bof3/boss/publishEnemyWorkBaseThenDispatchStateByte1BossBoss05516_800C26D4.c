#include "bof3/bof3.h"

/* The 0x118-byte reserve enemy work records the main program keeps at
 * 0x801EB630; only the base address (record 0) is used here. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back, for example 0x801E24F8 and 0x801E2284 of
 * BIN/BATTLE/BATTLE.EMI#03. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/*
 * This overlay's handler run at 0x800C35D4: sixteen code pointers, entry 0 =
 * 0x800C204C through entry 15 = 0x800C2FFC. This dispatcher indexes it from
 * 0x800C35F8, so its base is entry 9 (0x800C2728), and the entries it can reach
 * inside the run are entries 9 through 15: 0x800C2728, 0x800C2888, 0x800C29E8,
 * 0x800C2B48, 0x800C2CD4, 0x800C2E74 and 0x800C2FFC. The Splat `textbin` row
 * T_800C35D4 places the words, so the run is only referenced here. The
 * neighbouring base 0x800C35EC (entry 6) belongs to the sibling dispatcher
 * dispatchIndexedHandlerBossBoss05516_800C22F0, which indexes the same run.
 */
extern void (*D_800C35F8[])(void); /* @source 0x800C35F8 @kind data */

/* @source 0x800C26D4
 * @behavior Overlay step dispatch that first publishes work: stores the address
 * of the reserve enemy work records 0x801EB630 into the current enemy work cell
 * 0x801EB4E8, then reads the state byte at +1 of the work record published at
 * the scratchpad pointer cell 0x1F800044 and calls the handler pointer at that
 * index of this overlay's handler run based at 0x800C35F8. The index is not
 * bounded before the scaled table load, and the epilogue leaves $v0 untouched,
 * so this function's result is whatever the selected handler returns; no
 * in-payload caller consumes it. Only the address of the reserve work base is
 * published, and the scratchpad pointer load precedes frame creation in the
 * original. Neither this function's address nor its table base appears in any
 * word of this payload, so it is an overlay entry reached from outside the
 * payload; its prologue and return path alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishEnemyWorkBaseThenDispatchStateByte1BossBoss05516_800C26D4(void) {
  D_801EB4E8 = D_801EB630;
  D_800C35F8[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
