#include "bof3/bof3.h"

typedef void (*Boss03816Handler)(void);

/* The 0x800C1C90 handler run this dispatcher selects from, the 0x80-byte offset
 * inside the 0x8C-byte in-image data row the Splat `textbin` row D_800C1C10
 * covers (0x800C1C10 to 0x800C1C9C). The run holds exactly the three 8-byte
 * inert handlers of this overlay -- 0x800C1A1C, 0x800C1A24 and 0x800C1A2C --
 * and its last entry is the last word of that row, so this declaration binds
 * only the word the scaled index is added to and the row stays one row. The
 * generated undefined_syms_auto.txt row for D_800C1C90 already names it, so this
 * declaration needs no map row.
 */
extern Boss03816Handler D_800C1C90[]; /* @source 0x800C1C90 @kind data */

/* @source 0x800C19E4
 * @behavior Byte-indexed step dispatcher of this overlay's work state machine:
 * masks the index argument with 0xFF, scales it by four and calls the handler
 * selected from the three-word run at 0x800C1C90 -- the inert handlers
 * 0x800C1A1C, 0x800C1A24 and 0x800C1A2C, so every index reaches a no-op body --
 * and the selected handler's result stays in the return register. The original
 * bounds nothing before the scaled load, and the slots after the index mask and
 * after the table load stay nops. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call. No `jal` in
 * this payload names 0x800C19E4, and its address appears only in the step-setup
 * publisher 0x800C1980, which stores it into +0xE4 of the current enemy work
 * record from 0x801EB4E8, so that caller reaches it through the stored slot and
 * supplies the index; the argument role stays structural. The 0x38-byte body is
 * the same body as the already-exact sibling
 * dispatchIndexedHandler2 of this payload: one payload cmp
 * over both spans leaves a single differing byte, the %lo of the table load at
 * +0x18, 0x1C90 here against 0x1C54 there, every other byte equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandler3(u8 index) {
  D_800C1C90[index]();
}
