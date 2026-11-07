#include "bof3/bof3.h"

/* @source 0x80145988 @kind unknown */
extern volatile u8 paletteStageSerial;

/* @source 0x801FB0B8
 * @behavior Clamps one 16-entry palette window channel by channel and then bumps
 * the shared palette-stage serial. The intensity argument arrives in $a0 and is
 * narrowed with `andi 0xFF`; a second `andi 0xFFFF` keeps the same value as the
 * unsigned limit compared with `sltu`. The outer loop walks the palette source
 * staging block 0x80033800 up to the exclusive end 0x80033820 (signed `slt`
 * compare, source halfword read once per entry with `lhu`) and the destination
 * shadow block 0x80037800 is addressed as its `lui`/`ori` base plus a running
 * 2-byte offset, rematerialised inside the outer loop. For every entry the body
 * splits the low 15 bits into three 5-bit components with a running 0/5/10 shift
 * (`srav` plus `andi 0x1F`), replaces with the intensity any component the limit
 * compares below, and packs the components back through a 16-bit accumulator
 * (`sll 5` into the product word, `or` with the component, truncating copy back
 * into the carried word). The packed word is stored unconditionally (the store
 * fills the `beqz` delay slot of its own zero test) and a non-zero word is
 * re-stored with bit 15 set. The tail reads the palette-stage serial byte at
 * 0x80145988, adds one and stores it back with the load delay left as `nop`.
 * Leaf: no frame, `jr $ra` plus `nop`, no argument register read besides $a0.
 * The two `jal` sites in func_801FAFA0 (0x801FB04C and 0x801FB084) each forward a
 * byte-masked intensity derived from the scratchpad countdown at 0x1F800000, and
 * the immediately following 0x801FB164 bumps the same serial after copying this
 * function's output window. This range is byte-identical to
 * clampPaletteChannels (0x801F83B0 in emi/scenario/scena16/00), which supplies
 * the statement order; its recorded residual is the register rotation this lift
 * does not have.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The decisive shape is the carried word's width. Declaring the accumulator
 * `u16` and writing the pack as one truncating assignment keeps the loop's
 * carried value narrow, so the `move t0,a3` back into it is the truncating copy
 * and the later `sll v0,t0,5` needs no zero-extension. A `u32` accumulator (the
 * scena16 spelling) emits the same body but lets the cse pass after loop
 * optimisation replace the accumulator's later uses with the product word,
 * rotating the t-register pool and matching 51 percent. Widening the shift
 * operand, masking the shifted value with 0xFFFF, multiplying by 0x20 and
 * keeping a separate product local all measured 41/44 or worse at 176 bytes; a
 * `u16` product local measured 37/44. Recomputing the limit mask inside the
 * channel loop is the spelling that let the entry block order match: with a
 * hoisted limit assignment the same body measured 19/43 at the same size. Live
 * audit is instruction- and byte-exact: 43/43 instructions, 172 bytes.
 */
void clampPaletteChannelsScenarioScena1300_801FB0B8(u32 intensity) {
  const u16 *src;
  const u16 *srcEnd;
  u32 offset;
  u32 limit;
  u16 packed;
  s32 color;
  u32 component;
  s32 i;
  u16 *dst;

  intensity &= 0xffu;
  limit = intensity & 0xffffu;
  offset = 0u;
  src = PSX_PTR(const u16, 0x80033800u);
  srcEnd = PSX_PTR(const u16, 0x80033820u);

  do {
    packed = 0u;
    i = 0;
    color = *src;
    for (; i < 3; i++) {
      component = (u32)(color >> (i * 5)) & 0x1fu;
      limit = intensity & 0xffffu;
      if (limit < component) {
        component = intensity;
      }
      packed = (u16)((packed << 5) | component);
    }

    dst = (u16 *)((u8 *)PSX_PTR(u16, 0x80037800u) + offset);
    *dst = (u16)packed;
    if ((packed & 0xffffu) != 0u) {
      *dst = (u16)(packed | 0x8000u);
    }

    src++;
    offset += 2u;
  } while ((s32)src < (s32)srcEnd);

  paletteStageSerial = (u8)(paletteStageSerial + 1u);
}
