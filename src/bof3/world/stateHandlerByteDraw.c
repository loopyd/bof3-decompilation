#include "bof3/bof3.h"

extern u8 D_80144F28;

/* Scratchpad work-object pointer cell, bound in this target's map at
 * 0x1F800044. The original reads the cell once inside the flag-set arm as
 * `lui $v1,0x1F80` / `lw $v1,0x44($v1)` and keeps no saved register for the
 * cell address across the calls. */
extern u8* g_world02Area11113Work; /* @source 0x1F800044 @kind data */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_801F3244(void);

/* @source 0x801F319C
 * @behavior State handler 1 of the byte-2 dispatch table based at 0x801F489C
 *           for the work record published at the scratchpad pointer cell
 *           0x1F800044. It probes flag 0x28 of the main-RAM bank at
 *           0x80144F28 through the shared flag tester at 0x8015B5D4: when
 *           that flag reads set it advances the record's dispatch byte at
 *           offset 0x02, so the next frame dispatches handler 2, and in
 *           either case it then calls the overlay draw routine at
 *           0x801F3244. Takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps $ra at 0x10 across the two calls.
 *
 * Name evidence: the flag index is the `addiu $a1,$zero,0x28` argument the
 * original pairs with the `lui/addiu $a0,%hi(D_80144F28)` bank address; the
 * advanced field is the `lbu $v0,0x2($v1)` / `addiu $v0,$v0,1` / `sb` triple
 * on the cell the payload's two dispatchers read at offsets 0x01 and 0x02;
 * the trailing call is the `jal 0x801F3244` both neighbours issue; and the
 * three in-payload pointers 0x801F3130/0x801F319C/0x801F31F0 at 0x801F489C,
 * 0x801F48A0 and 0x801F48A4 make this entry 1 of that table.
 *
 * Shape: seeded from the original bytes, since a payload scan across
 * out/binaries found no byte-identical twin (one exact hit, itself, and none
 * with the two `jal` words wildcarded), so the clone, carve and data levers do
 * not apply and the range carries its own prologue and `jr $ra`. The
 * declaration style, the cell spelling and the single-read increment follow
 * the exact sibling at 0x801F3130, whose else arm holds the same increment and
 * draw pair; this body drops that sibling's set arm and moves the draw call to
 * the join, as the original shows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateHandlerByteDraw(void) {
  if (func_8015B5D4((u32)&D_80144F28, 0x28) != 0) {
    g_world02Area11113Work[2] = g_world02Area11113Work[2] + 1;
  }
  func_801F3244();
}
