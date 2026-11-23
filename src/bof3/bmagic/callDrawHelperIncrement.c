#include "bof3/bof3.h"

/* Draw helper at 0x548 of this payload: it queries GetGraphType, sets a draw
 * mode through SetDrawMode and emits translucent gouraud quads with SetPolyG4
 * and SetSemiTrans. The original passes no argument (the jal delay slot is a
 * nop) and drops the result, so the prototype keeps the narrow arity. */
extern void func_801EF148(void); /* @source 0x801EF148 @kind function */

/* Shared per-frame counter in main RAM, outside this overlay's image. The
 * shared map owns the address (config/targets/shared/symbols.txt), so this
 * target needs no row for it; its low bit is the per-frame phase the bmagic
 * sibling lifts read, and it is relocated here as %hi/%lo. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map as g_battle_work. A byte store through the loaded pointer may alias the
 * cell itself, so the compiler re-materializes the address (lui $vN,0x1F80 /
 * lw $vN,68($vN)) once for the byte-0xA group, once for the byte-9 increment
 * and once for the byte-1 advance, which is where the original puts those
 * three load pairs. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEEDC
 * @behavior Entry 2 of the six-word work-byte-1 handler run at 0x801EEC04
 *           (0x801EEC94, 0x801EEE68, 0x801EEEDC, 0x801EEF80, 0x801EF068,
 *           0x801EF114), which dispatchWorkByte1HandlerBmagicMagic11003_801EEC1C
 *           copies into its frame and jalr's for the value of work byte 1, so
 *           this body runs while that byte holds 2 and no jal in the payload
 *           reaches this address. It first calls the overlay's draw helper at
 *           0x801EF148 with no arguments. It then tests bit 0 of the shared
 *           per-frame counter at 0x80143E6C (lui %hi / lw %lo / andi 0x1) and
 *           runs the rest only on the phases where that bit is set: it adds
 *           two to work byte 0xA at byte width while that byte is below 0x10
 *           (lbu / sltiu / beqz / addiu 2 / sb), then increments work byte 9
 *           and, when the truncated count equals 0x10, increments work byte 1
 *           (lbu / addiu 1 / sb, andi 0xFF / addiu 0x10 / bne), which steps the
 *           dispatcher to entry 3. Both later groups re-read the scratchpad
 *           pointer cell at 0x1F800044 because the byte store before them may
 *           alias the cell. It takes no arguments, returns nothing, and uses a
 *           0x18-byte frame that saves only the return address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callDrawHelperIncrement(
    void) {
  u8 count;

  func_801EF148();
  if (D_80143E6C & 1) {
    if (g_battle_work[0xA] < 0x10) {
      g_battle_work[0xA] += 2;
    }
    count = g_battle_work[9] + 1;
    g_battle_work[9] = count;
    if (count == 0x10) {
      g_battle_work[1] += 1;
    }
  }
}
