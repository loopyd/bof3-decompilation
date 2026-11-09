#include "bof3/bof3.h"

typedef void (*BmagicMagic05703PhaseHandler)(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound at that address by
 * this target's map as g_battle_work, the name the sibling lifts of this module
 * use. The original re-materialises the cell with its own lui 0x1F80 / lw 0x44
 * pair at every access group, because a store through the loaded pointer may
 * alias the cell, so this lift reaches the cell through the map symbol and lets
 * each group that stores re-read it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay image.
 * This target's generated undefined_syms_auto.txt already binds the address,
 * so the name needs no map row. This body tests bit 0 of record byte 0 and
 * increments record byte 2; both offsets stay anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared status byte in main RAM that receives the mode constant 2; the same
 * cell the other bmagic and battle lifts publish their mode through. Written
 * only, so its meaning beyond the literal is not established. */
extern u8 D_80149333; /* @source 0x80149333 @kind data */

/* Shared halfword counter in main RAM that receives twice the work object's
 * signed word 0x10, the cell the sibling bmagic ramp handlers scale their ramp
 * into. Written only. */
extern u16 D_8014932A; /* @source 0x8014932A @kind data */

/* Stage reload-count byte table at 0x801EFADC, the three-byte tail segment of
 * this overlay image. This body indexes it with work byte 0x09 after advancing
 * that byte, and the sibling lift
 * clearWorkByte9SetWorkByteAFromStageTableInvokeHelperAt100ClearWorkWord10And
 * IncrementWorkByte2BmagicMagic05703_801EF5BC reads it the same way, so byte
 * 0x09 is the stage index of a three-entry sequence and the selected byte is
 * the per-stage frame countdown that work byte 0x0A holds. What the three
 * entries mean beyond that indexing is not established, so the table keeps its
 * address-anchored name. */
extern u8 D_801EFADC[]; /* @source 0x801EFADC @kind data */

/* One-argument overlay helper whose argument every caller in this tree sets to
 * an encoded constant, here 0x100. Bound by this target's generated
 * undefined_funcs_auto.txt row. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* The two phase handlers this body seeds into its local table, both following
 * spans of this overlay: 0x801EF7A0 is lifted as
 * decrementWorkWord10AndAdvanceWorkByte3WhenBelowMinus2BmagicMagic05703_801EF7A0
 * and 0x801EF7E0 is still generated assembly. Each address reaches the frame
 * through its own lui %hi / addiu %lo pair, so both need a declaration here. */
void decrementWorkWord10AndAdvanceWorkByte3WhenBelowMinus2BmagicMagic05703_801EF7A0(
    void);
void func_801EF7E0(void);

/* @source 0x801EF620
 * @behavior Per-frame phase dispatcher and publisher of this overlay's work
 *           sequence: the 0x180-byte span is entry 1 of the three-word code
 *           pointer table at 0x801EEC38 (0x801EF5BC, 0x801EF620, 0x801EF81C),
 *           which the state handler func_801EF55C copies into its own frame and
 *           indexes with work byte 0x02, so no jal word in the payload names
 *           this address. It first seeds the two phase handlers 0x801EF7A0 and
 *           0x801EF7E0 into a two-entry handler array in its own 0x20-byte frame
 *           and invokes the entry selected by work byte 0x03, with no arguments
 *           and no bounds check; the pair ramps work word 0x10 between -3 and
 *           zero and takes byte 0x03 back below zero.
 *           It then reads the work object again and counts down work byte 0x0A,
 *           the per-stage frame countdown: a countdown already at 0xFF is left
 *           alone and the stage block is skipped, otherwise the byte is
 *           decremented and the block runs only when the stored value truncates
 *           to zero. On zero it holds one work-observer pointer across an
 *           increment of the stage index byte 0x09 and a reload of the countdown
 *           from the stage table at 0x801EFADC indexed by the new stage, then
 *           invokes func_8015D71C with the selector 0x100, the countdown store
 *           filling the call delay slot. After the call it compares the stage
 *           index with 3: below 3 it clears phase byte 0x03 so the next frame
 *           starts the ramp again, and at 3, the last stage, it increments byte
 *           0x02 of the shared record at 0x801EB4E0 and pins the countdown to
 *           0xFF, which the guard above leaves alone on every later frame.
 *           Whenever bit 0 of record byte 0 is clear it reads the object once to
 *           increment work byte 0x02, the index of the same three-word state
 *           table, so the next frame runs entry 2, reads it a second time to
 *           clear phase byte 0x03, and clears work word 0x10 through the first
 *           of those two pointers. Last it publishes mode 2 into the shared
 *           status byte D_80149333 and twice the signed work word 0x10 into the
 *           shared halfword D_8014932A.
 *           It takes no arguments and returns nothing through the jr $ra at
 *           0x801EF798 inside a 0x20-byte frame that keeps $ra at 0x18 and the
 *           handler array at 0x10. The two pointer reads of the last block are
 *           kept as separate locals because the original keeps them in separate
 *           registers across the word store: the first read reaches the byte
 *           0x02 increment and the word 0x10 store, the second only the byte
 *           0x03 clear. Work bytes 0x02, 0x03, 0x09 and 0x0A, work word 0x10
 *           and record bytes 0 and 2 stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPhaseCountDownStageTimerAdvanceStageTableInvokeHelperAt100AndPublishWorkWord10ToHalfword8014932ABmagicMagic05703_801EF620(
    void) {
  u8 stage;
  s32 ramp;
  BmagicMagic05703PhaseHandler handlers[2] = {
      decrementWorkWord10AndAdvanceWorkByte3WhenBelowMinus2BmagicMagic05703_801EF7A0,
      func_801EF7E0};

  handlers[g_battle_work[3]]();
  if (g_battle_work[0x0A] != 0xFF && --g_battle_work[0x0A] == 0) {
    u8 *work;

    work = g_battle_work;
    stage = (u8)(work[0x09] + 1);
    work[0x09] = stage;
    work[0x0A] = D_801EFADC[stage];
    func_8015D71C(0x100);
    if (g_battle_work[0x09] == 3) {
      D_801EB4E0[0x02] += 1;
      g_battle_work[0x0A] = 0xFF;
    } else {
      g_battle_work[3] = 0;
    }
  }
  if ((D_801EB4E0[0] & 1) == 0) {
    u8 *work;
    u8 *phaseWork;

    work = g_battle_work;
    work[0x02] += 1;
    phaseWork = g_battle_work;
    *(s32 *)(work + 0x10) = 0;
    phaseWork[3] = 0;
  }
  ramp = *(s32 *)(g_battle_work + 0x10);
  D_80149333 = 2;
  D_8014932A = (u16)(ramp << 1);
}
