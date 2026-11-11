#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared main-RAM cells this overlay reaches: the signed reference pair the two
 * 16.16 positions are measured against, the frame-buffer index byte, the
 * render-cell table entry run and the primitive cursor. */
extern s16 D_80149320;
extern s16 D_80149322;
extern u8 D_80143D44;
extern u8 D_80142CC4[];

/* @source 0x801F8034
 * @behavior Maps two 16.16 position words onto one render-cell index: the index
 * is the high half of the first position minus the signed reference halfword at
 * D_80149320, plus the high half of the second position minus the reference
 * halfword at D_80149322, plus one per nonzero fractional half of either
 * position, plus the signed byte argument and one more when the second
 * position carries a fractional half. An index of 0x38 or more leaves the body
 * untouched, and so does a primitive arena that has no room for the requested
 * byte count below D_80143D44 * 0x9000 + 0x80028FCC. Otherwise the cell entry
 * at D_80142CC4 + ((D_80143D44 + 4) << 3) + index * 0x30 is read as a pointer,
 * the low 24 bits of its leading word are replaced with the low 24 bits of the
 * primitive cursor while its top byte stays, the cursor advances by that byte
 * count, and the entry is republished as the fifth argument's word. No call, no
 * scratchpad access and no stack frame. No payload holds a jal to this address
 * or a word equal to it, so its argument roles stay structural.
 * @status partial
 * @match 88.33
 * @residual Live gate: 53 of 60 instructions at the payload size of 240 bytes.
 *   Arithmetic, both position tests, the bounds test, the mask/or pair, every
 *   store and the return all match. The first difference is the register of the
 *   accumulated index: the build writes it in $t0 where the original carries it
 *   in $v1, and the drift that follows puts the signed byte step in $v1 instead
 *   of $a0 and adds the one to the index rather than to that step. Local/global
 *   allocation dumps of this source show why: the high-half difference and the
 *   index accumulate into one pseudo assigned $t0, whose range spans the $v1 and
 *   $a0 uses of the second position, while the original keeps three levels in
 *   $t0, $v1 and $a2. Naming the sum as its own variable does split it, but then
 *   the entry block schedules the second position ahead of the first, which
 *   measured 47 of 60; a single-expression sum measured 41 of 60 with four bytes
 *   over. The last difference is the cursor store, which this source schedules
 *   two slots ahead of the original's.
 */
void setRenderCellCursorForMappedPositionScenarioScena0400_801F8034(
    u32 pos0, u32 pos1, s8 extra, s32 advance, s32 *packet) {
  u32 delta;
  u32 sum;
  u32 dy;
  s32 step;
  u32 offset;
  s32 *cell;
  u8 *primitive;

  sum = ((s32)pos0 >> 16) - D_80149320;
  dy = ((s32)pos1 >> 16) - D_80149322;
  if (pos0 & 0xFFFF) {
    dy++;
  }
  sum += dy;
  step = extra;
  delta = sum + ((pos1 & 0xFFFF) ? step + 1 : step);

  if (delta >= 0x38) {
    return;
  }

  primitive = g_PrimCursor;
  if ((u8 *)(D_80143D44 * 0x9000 + 0x80028FCC) > (primitive + advance)) {
    offset = ((D_80143D44 + 4) << 3) + delta * 0x30;
    cell = *(s32 **)(D_80142CC4 + offset);
    g_PrimCursor = primitive + advance;
    *cell = (*cell & 0xFF000000) | ((u32)primitive & 0xFFFFFF);
    *(s32 **)(D_80142CC4 + offset) = packet;
  }
}
