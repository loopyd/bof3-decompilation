#include "bof3/bof3.h"

typedef void (*Boss00716Handler)(void);

/* Handler run this dispatcher selects from, based at 0x800C1DD0, the 0x58-byte
 * offset inside the data row the Splat `textbin` row D_800C1D78 places at the
 * end of this payload. Its first three words are this overlay's inert handlers
 * 0x800C18F4, 0x800C18FC and 0x800C1904, entry 3 is the step-setup publisher
 * 0x800C1950 and the words after it are the main-program handlers the same row
 * continues with, so this declaration binds only the word the scaled index is
 * added to and the row stays one row. */
extern Boss00716Handler D_800C1DD0[]; /* @source 0x800C1DD0 @kind data */

/* @source 0x800C18BC
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C1DD0 handler
 * run, so the selected handler's result stays in the return register. The
 * original bounds nothing before the scaled table load, and the slots after the
 * index shift and after the table load stay nops. It reads no state and writes
 * none; the 0x18-byte frame only keeps $ra at 0x10 across the indirect call. No
 * `jal` and no payload word names 0x800C18BC: the only in-payload reference is
 * the address the step-setup publisher at 0x800C1864 stores into +0xE4 of the
 * current enemy work record, so whoever calls that stored slot supplies the
 * index and this function's argument role stays structural. The body is
 * instruction-for-instruction the same shape as the sibling indexed dispatchers
 * dispatchIndexedHandlerBossBoss05016_800C18B8 and
 * dispatchIndexedHandlerBossBoss02513_800C1A68, differing only in the %hi/%lo
 * immediates of the table address the `lui $at` and the following `lw` carry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedTable1dd0(u8 index) {
  D_800C1DD0[index]();
}
