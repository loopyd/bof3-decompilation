#include "bof3/bof3.h"

/* Shared palette-stage serial byte: every palette helper of this family
 * publishes its completion into it, and this routine leaves the constant 1
 * there once the whole shadow block is reworked.
 * @source 0x80145988 @kind unknown
 */
extern u8 paletteStageSerial;

/* @source 0x801F7960
 * @behavior Overlay palette helper, no arguments and no return value: it
 * rewrites the whole 0x80037800 shadow palette block in place to grey. The
 * block is 0x20 rows of 0x200 bytes (0x100 halfwords each); for every halfword
 * the body splits the low 15 bits into its three 5-bit channels, adds them,
 * divides the sum by three with the 0xAAAAAAAB multiply, and replicates that
 * average into all three channel fields while keeping bit 15 of the source
 * word. After both loops it publishes 1 in the shared palette-stage serial.
 * Evidence for the shapes: the block base is a plain integer constant
 * (`lui`/`ori` into $t3, hoisted ahead of both loops) and the divisor is the
 * same kind of pair ($t2); the row base is remade per row as `andi 0xFF` /
 * `sll 9` / `addu` against that base, so the row index is a byte-wide value
 * scaled by the 0x200 row stride, while the cell address adds the scaled
 * column byte offset to the row base. The channel extract takes the red bits
 * straight off the load into the sum register, then one shared `andi 0xFFFF`
 * on the same loaded halfword feeds both the green and the blue shift (`srl
 * 5`, `srl 10`), which is the split the exact sibling scalePaletteChannels
 * (0x801F5388 in emi/world02/area082/13) uses for the same pack; the mask is
 * kept in its own local so it is born after the sum instead of being folded
 * into two inline masks. The loaded halfword is declared signed: `lhu`
 * zero-extends either way and the masks keep the shifts logical, so the
 * declared signedness only decides which of the three value locals takes the
 * low argument register. The pack writes bit 15 first, then the two shifted
 * averages through one reused temporary, and spells the closing `or` as
 * `average | packed` so the result lands in the average's own register, which
 * is what keeps the store ahead of the inner loop's counter test (the store's
 * value register is reused by that test, so the store cannot sink into the
 * branch delay slot). Both loops are bottom tested, the column counter
 * increments in the multiply's delay slot, and the tail store of the constant
 * 1 fills the outer branch's delay slot, so the serial is written once through
 * `lui $at,%hi` / `sb %lo($at)`. Leaf: no frame, no saved register, `jr $ra`
 * plus `nop`. No in-image jal targets the address; the only image word holding
 * it is one entry of a code-pointer run in SCENA07's payload (file offset
 * 0x71B0), so its callers are outside this overlay. The block and the serial
 * are the addresses used by copyPaletteBlock32AndAdvanceSerial and
 * clampPaletteChannels in emi/scenario/scena13/00.
 * @status exact
 * @match 100.00
 * @residual none
 */
void averagePaletteChannelsScenarioScena1400_801F7960(void) {
  u32 row;
  u32 col;
  u32 rowBase;
  u16* cell;
  s32 color;
  u32 rgb;
  u32 red;
  u32 average;
  u32 packed;
  u32 shifted;

  row = 0u;
  do {
    col = 0u;
    rowBase = 0x80037800u + (row & 0xffu) * 0x200u;
    do {
      cell = PSX_PTR(u16, (col & 0xffffu) * 2u + rowBase);
      color = *cell;
      red = color & 0x1fu;
      rgb = color & 0xffffu;
      average = (red + ((rgb >> 5) & 0x1fu) + ((rgb >> 10) & 0x1fu)) / 3u;
      packed = color & 0x8000u;
      shifted = average << 10;
      packed = packed | shifted;
      shifted = average << 5;
      packed = packed | shifted;
      *cell = (u16)(average | packed);
      col++;
    } while ((col & 0xffffu) < 0x100u);
    row++;
  } while ((row & 0xffu) < 0x20u);

  paletteStageSerial = 1u;
}
