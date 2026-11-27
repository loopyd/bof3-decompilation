#include "bof3/bof3.h"

typedef void (*Boss03716Handler)(void);

/* Handler run this dispatcher selects from, based at 0x800C19D4: word 17 of the
 * pointer run this payload's Splat `textbin` row resourceStepData places at 0x800C1990,
 * 0x44 bytes into that row. Its three words reach the end of this payload and are
 * this overlay's inert handlers 0x800C18E4, 0x800C18EC and 0x800C18F4, so this
 * declaration binds only the word the scaled index is added to and the row stays
 * one row. */
extern Boss03716Handler D_800C19D4[]; /* @source 0x800C19D4 @kind data */

/* @source 0x800C18AC
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C19D4 handler
 * run, so the selected handler's result stays in the return register. The
 * original bounds nothing before the scaled table load, and the slots after the
 * index shift and after the table load stay nops. It reads no state and writes
 * none; the 0x18-byte frame only keeps $ra at 0x10 across the indirect call. No
 * `jal` and no payload word names 0x800C18AC: the only in-payload reference is
 * the address the step-setup publisher
 * publishStepSetup
 * stores into +0xE4 of the
 * current enemy work record it loads from 0x801EB4E8, so whoever calls that
 * stored slot supplies the index and this function's argument role stays
 * structural. The body is instruction-for-instruction the same shape as the
 * sibling indexed dispatchers dispatchIndexedHandlerBossBoss00716_800C18BC and
 * dispatchIndexedHandlerBossBoss00113_800C1AF0, differing only in the %lo
 * immediate of the table load word the payload bytes carry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandler(u8 index) {
  D_800C19D4[index]();
}
