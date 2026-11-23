#include "bof3/bof3.h"

extern void stateTripleLevelTriple(
    void);
extern void decrementScratchLevelTriple(
    void);

/* @source 0x801F4AB4
 * @behavior Calls one of two local handlers,
 * stateTripleLevelTriple
 * and decrementScratchLevelTriple,
 * selected by byte +4 of the work record published at scratchpad slot 0x44.
 * Builds the handler array on the stack and passes no arguments. The selector
 * is unchecked, as in the original. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkLocalHandlers(void) {
  void (*handlers[2])(void) = {
      stateTripleLevelTriple,
      decrementScratchLevelTriple};

  handlers[SPAD_PTR_SLOT(u8, 0x44)[4]]();
}
