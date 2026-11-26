#include "bof3/bof3.h"

typedef void (*Boss03816Handler)(void);

/*
 * Handler run tail at 0x800C1C54, the base this dispatcher's scaled index is
 * added to: entry 0 = 0x800C1924, entry 1 = 0x800C192C and entry 2 = 0x800C1934
 * are the inert handlers of this overlay, and entry 3 = 0x800C1980 is the step
 * setup that publishes this overlay's step handler and raises its work state to
 * 2. The base is the eighteenth word of the thirty-five-word in-image code
 * pointer run the textbin row tableStepDataRow covers (0x800C1C10 to 0x800C1C9C), so
 * it binds as its own data symbol rather than a row boundary, as the run stays
 * one reviewed row. The generated undefined_syms_auto.txt row for D_800C1C54
 * already binds the word the scaled index is added to, so this declaration
 * needs no map row.
 */
extern Boss03816Handler D_800C1C54[]; /* @source 0x800C1C54 @kind data */

/* @source 0x800C18EC
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1C54
 * handler run tail, so the selected handler's result stays in the return
 * register. The original does not bound the index before the scaled load; the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call and the slots
 * after the index mask and after the table load stay nops. The payload holds
 * neither a jal nor a pointer word naming 0x800C18EC, so the overlay loader that
 * enters the payload is its caller and the argument role stays structural.
 * Same C shape as the sibling indexed dispatchers of this family, for example
 * dispatchIndexedHandlerBossBoss01416_800C18A0: the 0x38-byte spans differ in
 * one word only, the %lo of the dispatcher's table address in the scaled load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandler2(u8 index) {
  D_800C1C54[index]();
}
