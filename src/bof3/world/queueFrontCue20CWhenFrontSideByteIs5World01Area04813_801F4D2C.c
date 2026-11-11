#include "bof3/bof3.h"

/* Shared front-side byte of the resident executable. The shared map row
 * 0x80143F03 binds it, and the reviewed overlay annotations read it as the
 * front side the player stands on: the four route bytes at 0x801FCA5C are
 * keyed by it, and the world-map steps branch on the values 0, 1, 2, 4, 5 and
 * 6. This body only loads it, with `lbu`, so it stays u8. */
extern u8 D_80143F03;

/* Shared frontend cue dispatcher of the resident executable at 0x8015DF18; the
 * cue id arrives zero-extended in the whole argument word, as the reviewed
 * declaration in include/bof3/scenario/sce10eff_internal.h records. */
extern void func_8015DF18(u16 cue);

/* @source 0x801F4D2C
 * @behavior Requests frontend cue 0x20C through the shared frontend cue
 *   dispatcher 0x8015DF18 while the shared front-side byte 0x80143F03 reads 5,
 *   and does nothing otherwise. The saved return address occupies the single
 *   stack slot above the outgoing argument word, so the body keeps the plain
 *   0x18 frame. Takes no arguments and returns nothing.
 *
 * Name evidence: the only word of this payload that names 0x801F4D2C is the
 *   last word of the trailing descriptor run at payload offset 0x37F4, so no
 *   statement in this payload calls the range and the caller stays unnamed; the
 *   body itself carries the whole role. The guard reads the shared front-side
 *   byte 0x80143F03, and the single statement requests cue 0x20C, so the name
 *   states exactly those two facts.
 *
 * Shape: no payload of the 396 shipped binaries holds this 48-byte span — one
 *   scan of every payload found the single hit, in this target — so neither
 *   clone nor a byte-identical twin applies. The nearest sibling is
 *   emi/world03/area145/14 at payload offset 0x258: ten of its twelve words are
 *   equal to this span's, and the two that differ are the `lbu` low half that
 *   names the gated byte (0x80143BB0 there, 0x80143F03 here) and the `addiu`
 *   that materialises the cue id (0x209 there, 0x20C here). That sibling is
 *   still on its `asm` row, so its disassembly is original bytes and supplies
 *   the statement order, the operand forms and the 0x18 frame this seed takes;
 *   this target's own bytes write the guard constant 5, the gated byte and the
 *   cue id.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontCue20CWhenFrontSideByteIs5World01Area04813_801F4D2C(void) {
  if (D_80143F03 == 5) {
    func_8015DF18(0x20C);
  }
}
