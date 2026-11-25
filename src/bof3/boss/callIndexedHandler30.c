#include "bof3/bof3.h"

typedef void (*Boss03616Handler)(void);

/*
 * Handler run tail at 0x800C20B0, the base this dispatcher's scaled index is
 * added to: entry 0 = 0x800C1B5C and entry 1 = 0x800C1B64 (inert handlers),
 * entry 2 = 0x800C1B6C, entry 3 = 0x800C1EE4, the byte-2 work dispatcher of this
 * overlay, whose word is the last one of the run. The base is the thirty-second
 * word of the thirty-five-word in-image code pointer run the textbin row
 * D_800C2020 covers, so it binds as its own data symbol rather than a row
 * boundary, as the run stays one reviewed row.
 */
extern Boss03616Handler D_800C20B0[]; /* @source 0x800C20B0 @kind data */

/* @source 0x800C1D40
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C20B0
 * handler run tail, so the selected handler's result stays in the return
 * register. The original does not bound the index before the scaled load; the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call and the slots
 * after the index mask and after the table load stay nops. The payload holds
 * neither a jal nor a pointer word naming 0x800C1D40, so the overlay loader that
 * enters the payload is its caller. Same C shape as the sibling indexed
 * dispatcher callIndexedHandler35 of this target: the
 * two 0x38-byte spans differ in one word only, the %lo of this dispatcher's
 * table address in the scaled load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler30(u8 index) {
  D_800C20B0[index]();
}
