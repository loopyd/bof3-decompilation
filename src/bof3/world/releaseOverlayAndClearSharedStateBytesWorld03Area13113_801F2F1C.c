#include "bof3/bof3.h"

extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

void func_8015C058(void);

/* @source 0x801F2F1C
 * @behavior Calls the shared overlay release helper at 0x8015C058 and then
 *           writes zero to the three consecutive shared front-end state bytes
 *           at 0x801448EB, 0x801448EC and 0x801448ED. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 56-byte body is byte-identical to the span at 0x801F1684 in
 * emi/etc/commu00/00, payload offset 0x2A84: a cmp of the two extracted
 * payload spans compared equal, including the call word 0x0C057016, so that
 * span supplied the statement order, operand forms and declaration style.
 * The three cleared cells and the called helper all sit in the shared main
 * executable outside either overlay payload, so no image-local address is
 * carried over from the sibling. The immediately following same-target span
 * at 0x801F2F54 reads the middle byte back as a state selector and dispatches
 * on it, which fixes these three cells as the shared state tuple rather than
 * scratch.
 */
void releaseOverlayAndClearSharedStateBytesWorld03Area13113_801F2F1C(void) {
  func_8015C058();
  D_801448EB = 0;
  D_801448EC = 0;
  D_801448ED = 0;
}
