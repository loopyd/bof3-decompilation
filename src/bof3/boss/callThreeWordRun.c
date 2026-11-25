#include "bof3/bof3.h"

typedef void (*Boss00716Handler)(void);

/* Handler run this dispatcher selects from, based at 0x800C1E0C, the 0x94-byte
 * offset inside the data row the Splat `textbin` row D_800C1D78 places at the end
 * of this payload. The run holds exactly three words -- this overlay's inert
 * handler 0x800C19EC, the record-halfword clear 0x800C19F4 and the inert handler
 * 0x800C1A04 -- and the payload ends one word past the last entry, so this
 * declaration binds only the word the scaled index is added to and the row stays
 * one row. */
extern Boss00716Handler D_800C1E0C[]; /* @source 0x800C1E0C @kind data */

/* @source 0x800C19B4
 * @behavior Index-argument overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the three-word handler
 * run based at 0x800C1E0C, so the selected handler's result stays in the return
 * register. The original bounds nothing before the scaled table load, and the
 * slots after the index shift and after the table load stay nops. It reads no
 * state and writes none; the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call. No `jal` in this payload names 0x800C19B4, and its address
 * appears only in the step-setup publisher 0x800C1950, which stores it into
 * +0xE4 of the current enemy work record, so whoever calls that stored slot
 * supplies the index and this function's argument role stays structural. The
 * 56-byte body is instruction-for-instruction identical to the already-exact
 * sibling callIndexedTable1dd0 of this payload and to
 * dispatchIndexedHandlerBossBoss05016_800C18B8 and
 * dispatchIndexedHandlerBossBoss02513_800C1A68; each differs in the `%lo`
 * immediate of its table load at +0x18 alone, 0x1E0C here against 0x1DD0, 0x1B7C
 * and 0x31E4 there, every other word equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callThreeWordRun(u8 index) {
  D_800C1E0C[index]();
}
