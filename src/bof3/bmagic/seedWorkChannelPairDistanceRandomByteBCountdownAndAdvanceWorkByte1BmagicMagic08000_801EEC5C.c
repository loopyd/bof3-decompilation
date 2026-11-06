#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by every exact sibling of the bmagic overlay family. It stays a named cell
 * rather than a constant-address accessor because the original re-materialises
 * the base at each access (lui $v1,0x1F80 / lw $v1,68($v1)): five separate
 * lui/lw pairs appear in the body, one per store or call group, since a store
 * through the loaded pointer may alias the cell and every call clobbers it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared fixed-point channel pair in main RAM, outside this overlay's image.
 * The exact siblings seedWorkWordPairAndFlagsThenAdvanceWorkByte2
 * BmagicMagic13103_801F04C0 and seedMotionTintAndChannelFieldsThenAdvance
 * WorkByte2BmagicMagic00800_801F05FC copy the same two words into the same work
 * words 0x34/0x38, and 0x801F9644 commits the pair 0x260000/0x1B8000. Both names
 * are address-encoded, so Splat binds them with no target-map row. */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Distance helper of the main executable. The scenario and world lifts declare
 * the same two-argument, signed-halfword prototype and use it for the 0x34/0x38
 * work word pair: func_801D1034 publishes its result in work halfword 0x3E
 * exactly as this entry does. */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x801EEC5C
 * @behavior Work byte 1 state 0 of the MAGIC080 overlay: entry 0 of the
 *           two-entry handler table func_801EEC04 builds in its own frame
 *           (0x801EEC5C, 0x801EECE4) and dispatches on the scratchpad work
 *           object's state byte 1. It seeds the work object published at the
 *           pointer cell 0x1F800044: work word 0x34 takes the shared channel
 *           word at 0x80149308, work word 0x38 takes 0x8014930C through one
 *           value local that is also the helper's second argument, and work
 *           halfword 0x3E takes the signed func_8015477C distance of that pair.
 *           It then seeds work byte 0xB with the low six bits of rand() (the
 *           frame-mask byte the phase-1 handler tests), sets the countdown byte
 *           9 to 0x30 and advances work byte 1 with a byte-width read-modify-
 *           write, which selects entry 1 (0x801EECE4). Each group re-reads the
 *           cell, so the original keeps five lui/lw pairs; the 0x38 word store
 *           and the 0x3E halfword store each fill a call delay slot, and the
 *           0x18-byte frame only keeps $ra across the two calls. The function
 *           takes no arguments, returns nothing and reads or writes no other
 *           state. Statement order, operand forms and declaration style follow
 *           the exact sibling seedWorkWordPairAndFlagsThenAdvanceWorkByte2
 *           BmagicMagic13103_801F04C0 (same cell, same 0x34/0x38 word pair from
 *           the same two globals) and func_801D1034 (same value-local-then-call
 *           spelling for the 0x3E distance), with this body's own literals and
 *           the byte-1 state advance of func_801EEC7C's family. The mask 0x3F,
 *           the countdown 0x30 and the store offsets 0xB, 9, 0x3E and 1 appear
 *           only in this target's binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkChannelPairDistanceRandomByteBCountdownAndAdvanceWorkByte1BmagicMagic08000_801EEC5C(
    void) {
  u32 *work;
  s32 y;
  s16 distance;

  work = (u32 *)g_battle_work;
  work[13] = D_80149308[0];
  y = (s32)D_8014930C[0];
  work[14] = y;
  distance = func_8015477C((s32)work[13], y);
  *(u16 *)(g_battle_work + 0x3E) = (u16)distance;
  g_battle_work[0xB] = (u8)(rand() & 0x3F);
  g_battle_work[9] = 0x30;
  g_battle_work[1]++;
}
